#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "music_manager.h"
#include "music_player.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using Logger = std::shared_ptr<spdlog::logger>;

void log_music_info(const MusicEngine::Music &music, const Logger &logger) {
    logger->info("===== Music for Testing =====\n"
                 "  Title: {}\n"
                 "  Artist: {}\n"
                 "  Duration: {} seconds\n"
                 "  File Path: \"{}\"",
                 music.title.empty() ? "Unknown" : music.title, music.artist.empty() ? "Unknown" : music.artist,
                 music.duration, music.file_path.string());
}

void monitor_progress(const MusicEngine::MusicPlayer &player, const Logger &logger, int duration_secs) {
    if (player.get_state() != MusicEngine::PlayerState::Playing) {
        logger->warn("Player is not playing. Cannot monitor progress.");
        return;
    }
    for (int i = 0; i < duration_secs; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (player.get_state() != MusicEngine::PlayerState::Playing)
            break;
        double current_pos = player.get_current_position();
        double total_duration = player.get_duration();
        int percent = player.get_current_position_percent();
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << "  Progress: " << current_pos << "s / " << total_duration << "s ("
           << percent << "%)";
        logger->info(ss.str());
    }
}

int main() {
    spdlog::set_pattern("[%n] [%^%l%$] %v");
    auto logger = spdlog::stdout_color_mt("ComprehensiveTest");
    logger->set_level(spdlog::level::info);

    logger->info("--- MusicEngine Comprehensive Test Starting ---");

    // --- 步骤 1: 扫描音乐 ---
    auto &manager = MusicEngine::MusicManager::get_instance();
    const std::filesystem::path music_directory = "/home/hao/音乐"; // <--- 请修改为你的音乐文件夹路径
    manager.set_directory_paths({music_directory});
    manager.start_scan();
    while (manager.is_scanning())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto all_musics = manager.get_all_musics();
    if (all_musics.empty()) {
        logger->error("No music files found. Test cannot continue.");
        return 1;
    }
    logger->info("Scan finished. Found {} music file(s).", all_musics.size());


    // --- 选择第一首音乐进行测试 ---
    const auto &music_to_play = all_musics[0];
    log_music_info(music_to_play, logger);

    { // --- 创建一个新的作用域来管理第一个 player 实例的生命周期 ---
        logger->info("\n--- Starting Main Test Suite (Tests 1-4) ---");
        MusicEngine::MusicPlayer player;

        // --- [Test 1] 播放与进度 ---
        logger->info("\n--- [Test 1] Playback & Progress Reporting ---");
        player.play(music_to_play);
        monitor_progress(player, logger, 5);

        // --- [Test 2] 百分比跳转 ---
        logger->info("\n--- [Test 2] seek_percent() ---");
        player.seek_percent(50);
        monitor_progress(player, logger, 5);

        // --- [Test 3] 秒数跳转 ---
        double total_duration = player.get_duration();
        if (total_duration > 20.0) {
            logger->info("\n--- [Test 3] seek() ---");
            player.seek(total_duration - 15.0);
            monitor_progress(player, logger, 5);
        }

        // --- [Test 4] 边界情况 ---
        logger->info("\n--- [Test 4] Edge Cases ---");
        logger->info("[Action] Testing clamping with seek_percent(150)...");
        if (auto clamped = player.seek_percent(150))
            logger->info("[Result] Clamped to {}%", *clamped);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        logger->info("[Action] Testing seek on a stopped player...");
        player.stop(); // stop() 被调用，player 实例的生命周期即将结束
        if (!player.seek_percent(30))
            logger->info("[Result] PASSED: Seek correctly ignored.");
    } // --- 第一个 player 实例在这里被销毁 ---


    // --- [Test 5] 回调测试 ---
    logger->info("\n--- [Test 5] Playback Finished Callback ---");

    // 为这个独立的测试创建一个全新的、干净的 player 实例
    MusicEngine::MusicPlayer player_for_callback_test;

    std::promise<void> finished_promise;
    auto finished_future = finished_promise.get_future();
    player_for_callback_test.set_on_playback_finished_callback([&]() {
        logger->info("[Callback] Playback finished callback triggered!");
        finished_promise.set_value();
    });

    logger->info("[Action] Playing and seeking to 3 seconds before end...");
    player_for_callback_test.play(music_to_play);
    if (player_for_callback_test.get_duration() > 4.0) {
        player_for_callback_test.seek(player_for_callback_test.get_duration() - 3.0);
    }

    logger->info("[Main Thread] Waiting for callback...");
    finished_future.wait();
    logger->info("[Result] PASSED: Callback successfully notified main thread.");

    logger->info("\n--- All tests finished successfully! ---");
    return 0;
}
