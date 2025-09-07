#include <chrono>
#include <thread>
#include "MusicManager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace MusicEngine;

std::vector<std::filesystem::path> music_dirs = {"../music_test", "/home/hao/音乐"}; // Adjusted path for convention

// Helper function to print music information
void printMusicInfo(const Music &music, auto logger) {
    logger->info("===== Music Information =====\n"
                 "  Title: {}\n"
                 "  Artist: {}\n"
                 "  Album: {}\n"
                 "  File Path: \"{}\"\n"
                 "  Has Cover Art: {}\n"
                 "  Duration: {} seconds\n"
                 "  Year: {}",
                 music.title.empty() ? "Unknown" : music.title, music.artist.empty() ? "Unknown" : music.artist,
                 music.album.empty() ? "Unknown" : music.album, music.filePath.string(), (music.hasCoverArt ? "Yes" : "No"),
                 music.duration, music.year != 0 ? std::to_string(music.year) : "Unknown");
}

int main() {
    // --- spdlog Initialization ---
    spdlog::set_pattern("[%n] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::trace);

    // Create a logger for this example
    auto logger = spdlog::stdout_color_mt("example");
    logger->set_level(spdlog::level::info);

    logger->info("--- Music Player System Starting ---");

    auto &manager = MusicManager::getInstance();

    // Set the directories to scan
    manager.setDirectoryPath(music_dirs);

    // Set supported music file extensions (optional). Default is .mp3, .m4a, .flac, .wav, .ogg
    manager.setSupportedExtensions({".mp3", ".flac", ".wav", ".m4a", ".ogg", ".aac"});

    // Define a callback for when the scan finishes
    auto scanCallback = [&logger](size_t count) { logger->info("[Callback] Scan finished. Found {} musics.", count); };

    // Start the first scan
    logger->info("[Main Thread] Requesting MusicManager to start the first scan...");
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

    // Get all musics from the manager and print them
    logger->info("[Main Thread] Fetching the final music list from MusicManager:");
    auto allMusics = manager.getAllMusics();
    if (allMusics.empty()) {
        logger->info("The database is empty. The target directories might contain no supported music files.");
    } else {
        logger->info("Retrieved {} musics in total. Details below:", allMusics.size());
        for (const auto &music : allMusics) {
            printMusicInfo(music, logger);
        }
    }

    // --- Demonstrate Search Functionality ---
    std::string searchTerm = "Genshin"; // Example search term
    logger->info("Demonstrating search: looking for musics with '{}' in the title:", searchTerm);
    auto searchResults = manager.searchMusics(searchTerm);
    for (const auto &music : searchResults) {
        printMusicInfo(music, logger);
    }


    // === Demonstrate Export Functionality ===
    std::filesystem::path export_path = "../music_database_export.log";
    logger->info("[Main Thread] Attempting to export the database to '{}'...", export_path.string());
    if (manager.exportDatabaseToFile(export_path)) {
        logger->info("[Main Thread] Database export successful!");
    } else {
        logger->error("[Main Thread] Database export failed.");
    }
    
    // Print all fetched music filenames
    auto musicNames = manager.getMusicNames();
    logger->info("All scanned music filenames:");
    for (const auto &musicName : musicNames) {
        logger->info("  - {}", musicName);
    }


    logger->info("--- System Shutting Down ---");
    return 0;
}
