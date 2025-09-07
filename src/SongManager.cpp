#include "SongManager.h"
#include <algorithm>
#include <atomic>
#include <format>
#include <future>
#include <mutex>
#include <string>
#include "SongParser.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// Pimpl struct to hide private members from the public header.
struct SongManager::Impl {
    std::vector<Song> m_songDatabase;
    mutable std::mutex m_dbMutex;
    std::future<void> m_scanFuture;
    std::atomic<bool> m_isScanning{false};
    std::vector<std::filesystem::path> m_directoryPaths;
    std::shared_ptr<spdlog::logger> m_logger;

    // Supported music file extensions
    std::vector<std::string> supportedExtensions = {".mp3", ".m4a", ".flac", ".wav", ".ogg"};

    // Constructor for the Impl struct
    Impl() {}
};

SongManager::SongManager() : pimpl(std::make_unique<Impl>()) {
    // Initial state is not scanning
    pimpl->m_isScanning = false;

    // Initialize the SongManager logger
    pimpl->m_logger = spdlog::stdout_color_mt("SongManager");
    pimpl->m_logger->set_level(spdlog::level::info);
    pimpl->m_logger->info("SongManager initialized.");

    // Initialize the SongParser logger
    SongParser::logger_init();
}

// Destructor implementation
SongManager::~SongManager() {
    // If a background scan is still running when the program exits, wait for it to complete.
    if (pimpl->m_scanFuture.valid()) {
        pimpl->m_scanFuture.wait();
    }
}

SongManager &SongManager::getInstance() {
    static SongManager instance;
    return instance;
}

bool SongManager::startScan(const std::function<void(size_t)> &onScanFinished) {
    // Check if directory paths have been set
    if (pimpl->m_directoryPaths.empty()) {
        pimpl->m_logger->error("Error: Directory paths have not been set.");
        return false;
    }

    // Check if a scan is already in progress
    if (pimpl->m_isScanning) {
        pimpl->m_logger->warn("Warning: A scan is already in progress.");
        return false;
    }

    pimpl->m_isScanning = true; // Set the scanning flag

    // Copy the paths to ensure the async task uses a stable version
    auto pathsToScan = pimpl->m_directoryPaths;

    // Use std::async to launch an asynchronous task
    pimpl->m_scanFuture = std::async(std::launch::async, [this, pathsToScan, onScanFinished]() {
        pimpl->m_logger->info("Background scan started...");

        std::vector<Song> newDatabase;

        for (const auto &dirPath : pathsToScan) {
            pimpl->m_logger->info("Scanning directory: {}", dirPath.string());
            try {
                // recursive_directory_iterator must be used on a single path inside the loop
                for (const auto &entry : std::filesystem::recursive_directory_iterator(dirPath)) {
                    if (entry.is_regular_file()) {
                        // Log the file currently being processed
                        pimpl->m_logger->debug("Processing file: {}", entry.path().string());

                        std::string extension = entry.path().extension().string();
                        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                        // Check if the file extension is in the supported list
                        for (const auto &supExt : pimpl->supportedExtensions) {
                            if (extension == supExt) {
                                // Parse the file and add it to the database
                                if (auto songOpt = SongParser::createSongFromFile(entry.path())) {
                                    newDatabase.push_back(*songOpt);
                                }
                                break;
                            }
                        }
                    }
                }
            } catch (const std::filesystem::filesystem_error &e) {
                pimpl->m_logger->error("Filesystem error: {}", e.what());
            } catch (...) {
                pimpl->m_logger->error("An unknown error occurred while scanning directory: {}", dirPath.string());
            }
        }

        size_t count = newDatabase.size();
        pimpl->m_logger->info("Scan complete. Found {} songs.", count);

        {
            std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);
            pimpl->m_songDatabase = std::move(newDatabase);
        }

        // Invoke the callback function to notify completion
        if (onScanFinished) {
            onScanFinished(count);
        }

        pimpl->m_isScanning = false;
    });

    return true;
}

bool SongManager::isScanning() const {
    return pimpl->m_isScanning;
}

std::vector<Song> SongManager::getAllSongs() const {
    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);
    return pimpl->m_songDatabase;
}

std::vector<Song> SongManager::searchSongs(const std::string &query) const {
    if (pimpl->m_directoryPaths.empty()) {
        pimpl->m_logger->error("Error: Directory paths have not been set. Cannot perform search.");
        return {};
    }

    std::vector<Song> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);
    for (const auto &song : pimpl->m_songDatabase) {
        std::string lowerTitle = song.title;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);

        if (lowerTitle.find(lowerQuery) != std::string::npos) {
            results.push_back(song);
        }
    }
    return results;
}

std::vector<std::string> SongManager::getSongNames() const {
    std::vector<std::string> results;
    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex); // Ensure thread safety

    if (pimpl->m_directoryPaths.empty()) {
        pimpl->m_logger->warn("Warning: Directory paths not set, but returning names from current (possibly empty) database.");
    }
    
    for (const auto &song : pimpl->m_songDatabase) {
        results.push_back(song.filePath.filename().string());
    }

    return results;
}

void SongManager::setDirectoryPath(const std::filesystem::path &directoryPath) {
    pimpl->m_directoryPaths = {directoryPath};
}

void SongManager::setDirectoryPath(const std::vector<std::filesystem::path> &directoryPaths) {
    pimpl->m_directoryPaths = directoryPaths;
}

void SongManager::setDirectoryPath(std::initializer_list<std::filesystem::path> directoryPaths) {
    pimpl->m_directoryPaths = directoryPaths;
}

bool SongManager::exportDatabaseToFile(const std::filesystem::path &outputPath) const {
    pimpl->m_logger->info("Request to export database to file: {}", outputPath.string());

    // Dynamically create a logger specifically for file output
    std::shared_ptr<spdlog::logger> file_logger;
    try {
        // Use the filename as the logger name to ensure uniqueness.
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(outputPath.string(), true); // true = overwrite
        file_logger = std::make_shared<spdlog::logger>("database_exporter", file_sink);
        
        // Set a simple pattern for the file log, without logger name or level
        file_logger->set_pattern("%v");
        file_logger->set_level(spdlog::level::info);

    } catch (const spdlog::spdlog_ex &ex) {
        pimpl->m_logger->error("Failed to create log file for export: {}. Error: {}", outputPath.string(), ex.what());
        return false;
    }

    // Lock the database for thread-safe access
    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);

    if (pimpl->m_songDatabase.empty()) {
        pimpl->m_logger->warn("Database is empty. Nothing to export.");
        file_logger->info("--- Database is empty ---");
        return true; // The operation itself succeeded
    }

    // Format the current time as a string
    const auto now = std::chrono::system_clock::now();
    const std::string formatted_time = std::format("{:%Y-%m-%d %H:%M:%S}", now);

    file_logger->info("--- Song Database Export ---");
    file_logger->info("Total Songs: {}", pimpl->m_songDatabase.size());
    file_logger->info("Export Time: {}", formatted_time);
    file_logger->info("----------------------------\n");

    // Iterate through the database and format the output
    for (const auto &song : pimpl->m_songDatabase) {
        auto format_field = [](const std::string &value) { return value.empty() ? "Unknown" : value; };

        // Format the output string
        std::string song_info =
            fmt::format("Title: {}\n"
                        "Artist: {}\n"
                        "Album: {}\n"
                        "Genre: {}\n"
                        "Year: {}\n"
                        "Duration: {} seconds\n"
                        "File Path: {}\n"
                        "Cover Art Size: {} bytes\n"
                        "----------------------------",
                        format_field(song.title), format_field(song.artist), format_field(song.album),
                        format_field(song.genre), song.year == 0 ? "Unknown" : std::to_string(song.year),
                        song.duration, song.filePath.string(), song.coverArt.size());

        // Write to the file
        file_logger->info(song_info);
    }

    pimpl->m_logger->info("Database successfully exported to: {}", outputPath.string());
    return true;
}

void SongManager::setSupportedExtensions(const std::vector<std::string> &extensions) {
    if (extensions.empty()) {
        pimpl->m_logger->warn("Warning: Attempted to set an empty list of supported extensions. Keeping existing settings.");
        return;
    }

    // Clear and set the new list of extensions
    pimpl->supportedExtensions.clear();
    for (const auto &ext : extensions) {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        if (!lowerExt.empty() && lowerExt[0] != '.') {
            lowerExt = "." + lowerExt; // Ensure the extension starts with a dot
        }
        pimpl->supportedExtensions.push_back(lowerExt);
    }
    
    // Log the updated list
    std::string extensions_str;
    for (const auto& ext : pimpl->supportedExtensions) {
        extensions_str += ext + " ";
    }
    pimpl->m_logger->info("Supported file extensions updated to: {}", extensions_str);
}