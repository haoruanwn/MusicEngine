#include <chrono>
#include <thread>
#include "SongManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

std::vector<std::filesystem::path> music_dirs = {"../music_test", "/home/hao/Music"}; // Adjusted path for convention

// Helper function to print song information
void printSongInfo(const Song &song, auto logger) {
    logger->info("===== Song Information =====\n"
                 "  Title: {}\n"
                 "  Artist: {}\n"
                 "  Album: {}\n"
                 "  File Path: \"{}\"\n"
                 "  Cover Art Size: {} bytes\n"
                 "  Cover MIME Type: {}\n"
                 "  Duration: {} seconds\n"
                 "  Year: {}",
                 song.title.empty() ? "Unknown" : song.title, song.artist.empty() ? "Unknown" : song.artist,
                 song.album.empty() ? "Unknown" : song.album, song.filePath.string(), song.coverArt.size(),
                 song.coverArtMimeType.empty() ? "Unknown" : song.coverArtMimeType, song.duration,
                 song.year != 0 ? std::to_string(song.year) : "Unknown");
}

int main() {
    // --- spdlog Initialization ---
    spdlog::set_pattern("[%n] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::trace);

    // Create a logger for this example
    auto logger = spdlog::stdout_color_mt("example");
    logger->set_level(spdlog::level::info);

    logger->info("--- Music Player System Starting ---");

    auto &manager = SongManager::getInstance();

    // Set the directories to scan
    manager.setDirectoryPath(music_dirs);

    // Set supported music file extensions (optional). Default is .mp3, .m4a, .flac, .wav, .ogg
    manager.setSupportedExtensions({".mp3", ".flac", ".wav", ".m4a", ".ogg", ".aac"});

    // Define a callback for when the scan finishes
    auto scanCallback = [&logger](size_t count) { logger->info("[Callback] Scan finished. Found {} songs.", count); };

    // Start the first scan
    logger->info("[Main Thread] Requesting SongManager to start the first scan...");
    if (manager.startScan(scanCallback)) {
        logger->info("[Main Thread] Scan task started successfully.");
    }

    // Immediately try to start another scan while the first one is running
    logger->info("[Main Thread] Requesting another scan immediately while the first is running...");
    if (!manager.startScan(scanCallback)) {
        logger->warn("[Main Thread] Scan request was rejected as expected, because a task is already in progress.");
    }

    // Simulate the main thread's event loop waiting for the scan to finish
    logger->info("[Main Thread] Waiting for the scan to complete...");
    while (manager.isScanning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    logger->info("[Main Thread] isScanning() returned false. Scan has finished.");

    // Get all songs from the manager and print them
    logger->info("[Main Thread] Fetching the final song list from SongManager:");
    auto allSongs = manager.getAllSongs();
    if (allSongs.empty()) {
        logger->info("The database is empty. The target directories might contain no supported music files.");
    } else {
        logger->info("Retrieved {} songs in total. Details below:", allSongs.size());
        for (const auto &song : allSongs) {
            printSongInfo(song, logger);
        }
    }

    // --- Demonstrate Search Functionality ---
    std::string searchTerm = "Genshin"; // Example search term
    logger->info("Demonstrating search: looking for songs with '{}' in the title:", searchTerm);
    auto searchResults = manager.searchSongs(searchTerm);
    for (const auto &song : searchResults) {
        printSongInfo(song, logger);
    }


    // === Demonstrate Export Functionality ===
    std::filesystem::path export_path = "../song_database_export.log";
    logger->info("[Main Thread] Attempting to export the database to '{}'...", export_path.string());
    if (manager.exportDatabaseToFile(export_path)) {
        logger->info("[Main Thread] Database export successful!");
    } else {
        logger->error("[Main Thread] Database export failed.");
    }
    
    // Print all fetched song filenames
    auto songNames = manager.getSongNames();
    logger->info("All scanned song filenames:");
    for (const auto &songName : songNames) {
        logger->info("  - {}", songName);
    }


    logger->info("--- System Shutting Down ---");
    return 0;
}