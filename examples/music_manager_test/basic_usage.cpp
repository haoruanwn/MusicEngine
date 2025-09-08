#include <chrono>
#include <thread>
#include "music_manager.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace MusicEngine;

std::vector<std::filesystem::path> music_dirs = {"../music_test", "/home/hao/音乐"}; // Adjusted path for convention

// Helper function to print music information
void print_music_info(const Music &music, auto logger) {
    logger->info("===== Music Information =====\n" 
                 "  Title: {}\n" 
                 "  Artist: {}\n" 
                 "  Album: {}\n" 
                 "  File Path: \"{}\"\n" 
                 "  Has Cover Art: {}\n" 
                 "  Duration: {} seconds\n" 
                 "  Year: {}", 
                 music.title.empty() ? "Unknown" : music.title, music.artist.empty() ? "Unknown" : music.artist, 
                 music.album.empty() ? "Unknown" : music.album, music.file_path.string(), (music.has_cover_art ? "Yes" : "No"), 
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

    auto &manager = MusicManager::get_instance();

    // Set the directories to scan
    manager.set_directory_paths(music_dirs);

    // Set supported music file extensions (optional). Default is .mp3, .m4a, .flac, .wav, .ogg
    manager.set_supported_extensions({".mp3", ".flac", ".wav", ".m4a", ".ogg", ".aac"});

    // Define a callback for when the scan finishes
    auto scan_callback = [&logger](size_t count) { logger->info("[Callback] Scan finished. Found {} musics.", count); };

    // Start the first scan
    logger->info("[Main Thread] Requesting MusicManager to start the first scan...");
    if (manager.start_scan(scan_callback)) {
        logger->info("[Main Thread] Scan task started successfully.");
    }

    // Immediately try to start another scan while the first one is running
    logger->info("[Main Thread] Requesting another scan immediately while the first is running...");
    if (!manager.start_scan(scan_callback)) {
        logger->warn("[Main Thread] Scan request was rejected as expected, because a task is already in progress.");
    }

    // Simulate the main thread's event loop waiting for the scan to finish
    logger->info("[Main Thread] Waiting for the scan to complete...");
    while (manager.is_scanning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    logger->info("[Main Thread] is_scanning() returned false. Scan has finished.");

    

    // Get all musics from the manager and print them
    logger->info("[Main Thread] Fetching the final music list from MusicManager:");
    auto all_musics = manager.get_all_musics();
    if (all_musics.empty()) {
        logger->info("The database is empty. The target directories might contain no supported music files.");
    } else {
        logger->info("Retrieved {} musics in total. Details below:", all_musics.size());
        for (const auto &music : all_musics) {
            print_music_info(music, logger);
        }
    }

        // --- Test: get the first music`s cover art and print its size ---
    if (!all_musics.empty()) {
        const auto &first_music = all_musics.front();
        logger->info("[Test] Fetching cover art for: {}", first_music.file_path.string());
        auto cover = manager.get_cover_art(first_music);
        if (cover) {
            logger->info("[Test] Cover art size: {} bytes", cover->size());
        } else {
            logger->info("[Test] No cover art available for this music.");
        }
    }

    // --- Demonstrate Search Functionality ---
    std::string search_term = "Genshin"; // Example search term
    logger->info("Demonstrating search: looking for musics with '{}' in the title:", search_term);
    auto search_results = manager.search_musics(search_term);
    for (const auto &music : search_results) {
        print_music_info(music, logger);
    }


    // === Demonstrate Export Functionality ===
    std::filesystem::path export_path = "../music_database_export.log";
    logger->info("[Main Thread] Attempting to export the database to '{}'...", export_path.string());
    if (manager.export_database_to_file(export_path)) {
        logger->info("[Main Thread] Database export successful!");
    } else {
        logger->error("[Main Thread] Database export failed.");
    }
    
    // Print all fetched music filenames
    auto music_filenames = manager.get_music_filenames();
    logger->info("All scanned music filenames:");
    for (const auto &music_name : music_filenames) {
        logger->info("  - {}", music_name);
    }


    logger->info("--- System Shutting Down ---");
    return 0;
}