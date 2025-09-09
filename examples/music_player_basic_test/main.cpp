#include <chrono>
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
 * @brief 使用 spdlog 打印播放器当前状态
 * @param player 音乐播放器实例
 * @param logger spdlog 的 logger 实例
 */
void print_player_state(const MusicEngine::MusicPlayer &player, const Logger &logger) {
    std::string state_str;
    switch (player.get_state()) {
        case MusicEngine::PlayerState::Stopped:
            state_str = "Stopped";
            break;
        case MusicEngine::PlayerState::Playing:
            state_str = "Playing";
            break;
        case MusicEngine::PlayerState::Paused:
            state_str = "Paused";
            break;
    }
    logger->info(">> Player status is now: {}", state_str);
}

/**
 * @brief 使用 spdlog 打印音乐的详细信息
 * @param music 音乐对象
 * @param logger spdlog 的 logger 实例
 */
void log_music_info(const MusicEngine::Music &music, const Logger &logger) {
    logger->info("===== Music to be Played =====\n"
                 "  Title: {}\n"
                 "  Artist: {}\n"
                 "  File Path: \"{}\"",
                 music.title.empty() ? "Unknown" : music.title, music.artist.empty() ? "Unknown" : music.artist,
                 music.file_path.string());
}

int main() {
    // --- spdlog 初始化 ---
    // 设置日志格式: [时间戳] [logger名] [日志级别] 日志消息
    spdlog::set_pattern("[%n] [%^%l%$] %v");
    // 创建一个带颜色的、线程安全的控制台 logger
    auto logger = spdlog::stdout_color_mt("PlayerTest");
    // 设置日志级别为 info，更低级别的 trace, debug 将不会显示
    logger->set_level(spdlog::level::info);

    logger->info("--- MusicEngine Player Functionality Test Starting ---");

    // --- 步骤 1: 设置 MusicManager 并扫描音乐文件 ---
    logger->info("[Step 1] Initializing Music Manager...");

    auto &manager = MusicEngine::MusicManager::get_instance();

    // 将下面的路径修改为电脑上存放音乐的实际文件夹路径
    const std::filesystem::path music_directory = "/home/hao/音乐";

    if (!std::filesystem::exists(music_directory) || !std::filesystem::is_directory(music_directory)) {
        logger->error("[Setup] Music directory does not exist: {}", music_directory.string());
        logger->error("[Setup] Please edit the 'music_directory' variable in the source "
                      "file.");
        return 1;
    }

    manager.set_directory_paths({music_directory});
    logger->info("[Setup] Set music directory to: {}", music_directory.string());

    // 定义扫描完成后的回调函数，它也会使用我们的 logger
    auto scan_callback = [logger](size_t count) {
        logger->info("[Callback] Asynchronous scan finished. Found {} music file(s).", count);
    };

    logger->info("[Main Thread] Starting asynchronous scan...");
    if (!manager.start_scan(scan_callback)) {
        logger->warn("[Main Thread] Scan could not be started (perhaps one is already "
                     "running).");
    }

    // 等待扫描完成
    logger->info("[Main Thread] Waiting for scan to complete...");
    while (manager.is_scanning()) {
        // 这里可以使用 trace 级别的日志，如果设置为 trace 级别就能看到这条消息
        // logger->trace("Main thread polling is_scanning()...");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    logger->info("[Main Thread] Scan has finished.");

    auto all_musics = manager.get_all_musics();
    if (all_musics.empty()) {
        logger->error("No music files found. The test cannot continue.");
        return 1;
    }
    logger->info("Successfully retrieved {} music file(s) from the manager.", all_musics.size());

    // --- 步骤 2: 测试 MusicPlayer 的核心功能 ---
    logger->info("[Step 2] Testing Music Player Controls...");
    MusicEngine::MusicPlayer player;
    // 选择第一首歌进行测试
    const auto &music_to_play = all_musics[0];

    logger->info("--- Starting Playback Sequence ---");
    log_music_info(music_to_play, logger);
    print_player_state(player, logger);

    // 测试 Play
    logger->info("[Action] Calling play()...");
    player.play(music_to_play);
    print_player_state(player, logger);
    logger->info("Playing for 10 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // 测试 Pause
    logger->info("[Action] Calling pause()...");
    player.pause();
    print_player_state(player, logger);
    logger->info("Paused for 3 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 测试 Resume
    logger->info("[Action] Calling resume()...");
    player.resume();
    print_player_state(player, logger);
    logger->info("Resuming playback for 10 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // 测试 Stop
    logger->info("[Action] Calling stop()...");
    player.stop();
    print_player_state(player, logger);
    logger->info("--- Playback Sequence Finished ---");

    logger->info("--- Test finished successfully! ---");

    return 0;
}
