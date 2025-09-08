#include <chrono>
#include <iomanip> // 用于 std::fixed 和 std::setprecision
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// 核心 MusicEngine 头文件
#include "music_manager.h"
#include "music_player.h"

// spdlog 用于日志记录
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// 为 logger 定义一个更易读的别名
using Logger = std::shared_ptr<spdlog::logger>;

/**
 * @brief 使用 spdlog 打印音乐的详细信息
 * @param music 音乐对象
 * @param logger spdlog 的 logger 实例
 */
void log_music_info(const MusicEngine::Music &music, const Logger &logger) {
    logger->info("===== Music for Testing =====\n"
                 "  Title: {}\n"
                 "  Artist: {}\n"
                 "  Duration: {} seconds\n"
                 "  File Path: \"{}\"",
                 music.title.empty() ? "Unknown" : music.title, music.artist.empty() ? "Unknown" : music.artist,
                 music.duration, music.file_path.string());
}

/**
 * @brief 监控并打印播放进度
 * @param player 音乐播放器实例
 * @param logger spdlog 的 logger 实例
 * @param duration_secs 监控持续的秒数
 */
void monitor_progress(const MusicEngine::MusicPlayer &player, const Logger &logger, int duration_secs) {
    if (player.get_state() != MusicEngine::PlayerState::Playing) {
        logger->warn("Player is not playing. Cannot monitor progress.");
        return;
    }

    for (int i = 0; i < duration_secs; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        double current_pos = player.get_current_position();
        double total_duration = player.get_duration();
        int percent = player.get_current_position_percent();

        // 使用 iomanip 格式化输出，确保小数位对齐
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << "  Progress: " << current_pos << "s / " << total_duration
           << "s (" << percent << "%)";
        logger->info(ss.str());
    }
}

int main() {
    // --- spdlog 初始化 ---
    spdlog::set_pattern("[%n] [%^%l%$] %v");
    auto logger = spdlog::stdout_color_mt("SeekTest");
    logger->set_level(spdlog::level::info);

    logger->info("--- MusicEngine Seek & Progress Test Starting ---");

    // --- 步骤 1: 设置 MusicManager 并扫描音乐文件 ---
    logger->info("[Step 1] Initializing Music Manager...");

    auto &manager = MusicEngine::MusicManager::get_instance();
    const std::filesystem::path music_directory = "/home/hao/音乐"; // <--- 请修改为你的音乐文件夹路径

    if (!std::filesystem::exists(music_directory) || !std::filesystem::is_directory(music_directory)) {
        logger->error("[Setup] Music directory does not exist: {}", music_directory.string());
        return 1;
    }
    manager.set_directory_paths({music_directory});

    logger->info("[Main Thread] Starting scan to find a music file...");
    manager.start_scan();
    while (manager.is_scanning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    auto all_musics = manager.get_all_musics();
    if (all_musics.empty()) {
        logger->error("No music files found in {}. Test cannot continue.", music_directory.string());
        return 1;
    }
    logger->info("Scan finished. Found {} music file(s).", all_musics.size());

    // --- 步骤 2: 准备播放器和测试音乐 ---
    MusicEngine::MusicPlayer player;
    // 选择第一首歌进行测试
    const auto &music_to_play = all_musics[0];
    log_music_info(music_to_play, logger);


    // --- 步骤 3: 测试播放与进度获取 ---
    logger->info("\n--- [Test 1] Testing Playback & Progress Reporting ---");
    logger->info("[Action] Calling play()...");
    player.play(music_to_play);
    logger->info("Playing for 5 seconds while monitoring progress...");
    monitor_progress(player, logger, 5);


    // --- 步骤 4: 测试 seek_percent() ---
    logger->info("\n--- [Test 2] Testing seek_percent() to middle of the song ---");
    logger->info("[Action] Calling seek_percent(50)...");
    if (auto actual_percent = player.seek_percent(50)) {
        logger->info("[Result] Seek request accepted. Target position is {}%.", *actual_percent);
    } else {
        logger->error("[Result] Seek request was ignored!");
    }
    logger->info("Continuing playback for 5 seconds to observe new progress...");
    monitor_progress(player, logger, 5);


    // --- 步骤 5: 测试 seek() ---
    double total_duration = player.get_duration();
    if (total_duration > 20.0) { // 确保歌曲足够长
        double seek_target_secs = total_duration - 15.0;
        logger->info("\n--- [Test 3] Testing seek() to 15 seconds before the end ---");
        logger->info("[Action] Calling seek({:.1f})...", seek_target_secs);

        if (auto actual_secs = player.seek(seek_target_secs)) {
            logger->info("[Result] Seek request accepted. Target position is {:.1f}s.", *actual_secs);
        } else {
            logger->error("[Result] Seek request was ignored!");
        }
        logger->info("Continuing playback for 5 seconds...");
        monitor_progress(player, logger, 5);
    } else {
        logger->warn("\n--- [Test 3] Skipped: Song is too short for this test case.");
    }


    // --- 步骤 6: 测试边界/无效情况 ---
    logger->info("\n--- [Test 4] Testing Edge Cases ---");
    // 6a: 测试超出范围的百分比
    logger->info("[Action] Calling seek_percent(150) to test clamping...");
    if (auto clamped_percent = player.seek_percent(150)) {
        logger->info("[Result] Request accepted. Input 150 was clamped to {}%.", *clamped_percent);
    } else {
        logger->error("[Result] Seek request was ignored!");
    }
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 短暂播放以确认

    // 6b: 测试在停止状态下跳转
    logger->info("[Action] Stopping player...");
    player.stop();
    logger->info("[Action] Calling seek_percent(30) on a stopped player...");
    if (auto result = player.seek_percent(30)) {
        logger->error("[Result] FAILED: Seek should be ignored on a stopped player, but wasn't.");
    } else {
        logger->info("[Result] PASSED: Seek request was correctly ignored as expected.");
    }


    logger->info("\n--- Test finished successfully! ---");
    return 0;
}