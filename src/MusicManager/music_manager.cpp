#include "music_manager.h"
#include <algorithm>
#include <atomic>
#include <format>
#include <future>
#include <mutex>
#include <string>
#include "cover_art_cache.hpp"
#include "music_parser.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


namespace music_engine {
    // Pimpl struct to hide private members from the public header.
    struct MusicManager::Impl {
        std::vector<Music> music_database_;
        mutable std::mutex db_mutex_;
        std::future<void> scan_future_;
        std::atomic<bool> is_scanning_{false};
        std::vector<std::filesystem::path> directory_paths_;
        std::shared_ptr<spdlog::logger> logger_;

        // Supported music file extensions
        std::vector<std::string> supported_extensions_ = { ".mp3", ".m4a", ".flac", ".wav", ".ogg" };

        // Constructor for the Impl struct
        Impl() {}
    };

    MusicManager::MusicManager() : pimpl_(std::make_unique<Impl>()) {
        // Initial state is not scanning
        pimpl_->is_scanning_ = false;

        // Initialize the MusicManager logger
        pimpl_->logger_ = spdlog::stdout_color_mt("MusicManager");
        pimpl_->logger_->set_level(spdlog::level::info);
        pimpl_->logger_->info("MusicManager initialized.");

        // Initialize the MusicParser logger
        music_parser::logger_init();
    }

    // Destructor implementation
    MusicManager::~MusicManager() {
        // If a background scan is still running when the program exits, wait for it to complete.
        if (pimpl_->scan_future_.valid()) {
            pimpl_->scan_future_.wait();
        }
    }

    MusicManager &MusicManager::get_instance() {
        static MusicManager instance;
        return instance;
    }

    bool MusicManager::start_scan(const std::function<void(size_t)> &on_scan_finished) {
        // Check if directory paths have been set
        if (pimpl_->directory_paths_.empty()) {
            pimpl_->logger_->error("Error: Directory paths have not been set.");
            return false;
        }

        // Check if a scan is already in progress
        if (pimpl_->is_scanning_) {
            pimpl_->logger_->warn("Warning: A scan is already in progress.");
            return false;
        }

        pimpl_->is_scanning_ = true; // Set the scanning flag

        // Copy the paths to ensure the async task uses a stable version
        auto paths_to_scan = pimpl_->directory_paths_;

        // Use std::async to launch an asynchronous task
        pimpl_->scan_future_ = std::async(std::launch::async, [this, paths_to_scan, on_scan_finished]() {
            pimpl_->logger_->info("Background scan started...");

            std::vector<Music> new_database;

            for (const auto &dir_path: paths_to_scan) {
                pimpl_->logger_->info("Scanning directory: {}", dir_path.string());
                try {
                    // recursive_directory_iterator must be used on a single path inside the loop
                    for (const auto &entry: std::filesystem::recursive_directory_iterator(dir_path)) {
                        if (entry.is_regular_file()) {
                            // Log the file currently being processed
                            pimpl_->logger_->debug("Processing file: {}", entry.path().string());

                            std::string extension = entry.path().extension().string();
                            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                            // Check if the file extension is in the supported list
                            for (const auto &sup_ext: pimpl_->supported_extensions_) {
                                if (extension == sup_ext) {
                                    // Parse the file and add it to the database
                                    if (auto music_opt = music_parser::create_music_from_file(entry.path())) {
                                        new_database.push_back(*music_opt);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                } catch (const std::filesystem::filesystem_error &e) {
                    pimpl_->logger_->error("Filesystem error: {}", e.what());
                } catch (...) {
                    pimpl_->logger_->error("An unknown error occurred while scanning directory: {}", dir_path.string());
                }
            }

            size_t count = new_database.size();
            pimpl_->logger_->info("Scan complete. Found {} musics.", count);

            {
                std::lock_guard<std::mutex> lock(pimpl_->db_mutex_);
                pimpl_->music_database_ = std::move(new_database);
            }

            // Invoke the callback function to notify completion
            if (on_scan_finished) {
                on_scan_finished(count);
            }

            pimpl_->is_scanning_ = false;
        });

        return true;
    }

    bool MusicManager::is_scanning() const { return pimpl_->is_scanning_; }

    std::vector<Music> MusicManager::get_all_musics() const {
        std::lock_guard<std::mutex> lock(pimpl_->db_mutex_);
        return pimpl_->music_database_;
    }

    std::vector<Music> MusicManager::search_musics(const std::string &query) const {
        if (pimpl_->directory_paths_.empty()) {
            pimpl_->logger_->error("Error: Directory paths have not been set. Cannot perform search.");
            return {};
        }

        std::vector<Music> results;
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

        std::lock_guard<std::mutex> lock(pimpl_->db_mutex_);
        for (const auto &music: pimpl_->music_database_) {
            std::string lower_title = music.title;
            std::transform(lower_title.begin(), lower_title.end(), lower_title.begin(), ::tolower);

            if (lower_title.find(lower_query) != std::string::npos) {
                results.push_back(music);
            }
        }
        return results;
    }

    std::vector<std::string> MusicManager::get_music_filenames() const {
        std::vector<std::string> results;
        std::lock_guard<std::mutex> lock(pimpl_->db_mutex_); // Ensure thread safety

        if (pimpl_->directory_paths_.empty()) {
            pimpl_->logger_->warn(
                    "Warning: Directory paths not set, but returning names from current (possibly empty) database.");
        }

        for (const auto &music: pimpl_->music_database_) {
            results.push_back(music.file_path.filename().string());
        }

        return results;
    }

    void MusicManager::set_directory_paths(const std::filesystem::path &directory_path) {
        pimpl_->directory_paths_ = {directory_path};
    }

    void MusicManager::set_directory_paths(const std::vector<std::filesystem::path> &directory_paths) {
        pimpl_->directory_paths_ = directory_paths;
    }

    void MusicManager::set_directory_paths(std::initializer_list<std::filesystem::path> directory_paths) {
        pimpl_->directory_paths_ = directory_paths;
    }

    bool MusicManager::export_database_to_file(const std::filesystem::path &output_path) const {
        pimpl_->logger_->info("Request to export database to file: {}", output_path.string());

        // Dynamically create a logger specifically for file output
        std::shared_ptr<spdlog::logger> file_logger;
        try {
            // Use the filename as the logger name to ensure uniqueness.
            auto file_sink =
                    std::make_shared<spdlog::sinks::basic_file_sink_mt>(output_path.string(), true); // true = overwrite
            file_logger = std::make_shared<spdlog::logger>("database_exporter", file_sink);

            // Set a simple pattern for the file log, without logger name or level
            file_logger->set_pattern("%v");
            file_logger->set_level(spdlog::level::info);

        } catch (const spdlog::spdlog_ex &ex) {
            pimpl_->logger_->error("Failed to create log file for export: {}. Error: {}", output_path.string(),
                                   ex.what());
            return false;
        }

        // Lock the database for thread-safe access
        std::lock_guard<std::mutex> lock(pimpl_->db_mutex_);

        if (pimpl_->music_database_.empty()) {
            pimpl_->logger_->warn("Database is empty. Nothing to export.");
            file_logger->info("--- Database is empty ---");
            return true; // The operation itself succeeded
        }

        // Format the current time as a string
        const auto now = std::chrono::system_clock::now();
        const std::string formatted_time = std::format("{:%Y-%m-%d %H:%M:%S}", now);

        file_logger->info("--- Music Database Export ---");
        file_logger->info("Total Musics: {}", pimpl_->music_database_.size());
        file_logger->info("Export Time: {}", formatted_time);
        file_logger->info("----------------------------\n");

        // Iterate through the database and format the output
        for (const auto &music: pimpl_->music_database_) {
            auto format_field = [](const std::string &value) { return value.empty() ? "Unknown" : value; };

            // Format the output string
            std::string music_info =
                    fmt::format("Title: {}\n"
                                "Artist: {}\n"
                                "Album: {}\n"
                                "Genre: {}\n"
                                "Year: {}\n"
                                "Duration: {} seconds\n"
                                "File Path: {}\n"
                                "Has Cover Art: {}\n"
                                "----------------------------",
                                format_field(music.title), format_field(music.artist), format_field(music.album),
                                format_field(music.genre), music.year == 0 ? "Unknown" : std::to_string(music.year),
                                music.duration, music.file_path.string(), (music.has_cover_art ? "Yes" : "No"));

            // Write to the file
            file_logger->info(music_info);
        }

        pimpl_->logger_->info("Database successfully exported to: {}", output_path.string());
        return true;
    }

    void MusicManager::set_supported_extensions(const std::vector<std::string> &extensions) {
        if (extensions.empty()) {
            pimpl_->logger_->warn(
                    "Warning: Attempted to set an empty list of supported extensions. Keeping existing settings.");
            return;
        }

        // Clear and set the new list of extensions
        pimpl_->supported_extensions_.clear();
        for (const auto &ext: extensions) {
            std::string lower_ext = ext;
            std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
            if (!lower_ext.empty() && lower_ext[0] != '.') {
                lower_ext = "." + lower_ext; // Ensure the extension starts with a dot
            }
            pimpl_->supported_extensions_.push_back(lower_ext);
        }

        // Log the updated list
        std::string extensions_str;
        for (const auto &ext: pimpl_->supported_extensions_) {
            extensions_str += ext + " ";
        }
        pimpl_->logger_->info("Supported file extensions updated to: {}", extensions_str);
    }

    std::shared_ptr<const std::vector<char>> MusicManager::get_cover_art(const Music &music) const {
        return CoverArtCache::get_instance().get_cover_art(music);
    }

} // namespace music_engine
