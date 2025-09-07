#include <chrono>
#include <thread>
#include "SongManager.h"
#include "opencv_show.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

std::vector<std::filesystem::path> music_dirs = {"../music_test", "/home/hao/音乐"};


// 用来打印歌曲信息
void printSongInfo(const Song &song, auto logger) {
    logger->info("===== 歌曲信息 =====\n"
                 "  标题: {}\n"
                 "  艺术家: {}\n"
                 "  专辑: {}\n"
                 "  文件路径: \"{}\"\n"
                 "  封面大小: {} 字节\n"
                 "  封面 MIME 类型: {}\n"
                 "  时长: {} 秒\n"
                 "  年份: {}",
                 song.title.empty() ? "未知" : song.title, song.artist.empty() ? "未知" : song.artist,
                 song.album.empty() ? "未知" : song.album, song.filePath.string(), song.coverArt.size(),
                 song.coverArtMimeType.empty() ? "未知" : song.coverArtMimeType, song.duration,
                 song.year != 0 ? std::to_string(song.year) : "未知");
}


int main() {
    // --- spdlog 初始化 ---
    spdlog::set_pattern("[%n] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::trace);


    // 创建一个example中用的日志类
    auto logger = spdlog::stdout_color_mt("example");
    logger->set_level(spdlog::level::info);


    logger->info("--- 音乐播放器系统启动 ---");

    auto &manager = SongManager::getInstance();

    // 设置要扫描的目录
    manager.setDirectoryPath(music_dirs);

    // 设置支持的音乐文件扩展名（可选）, 默认支持 .mp3, .m4a, .flac, .wav, .ogg
    manager.setSupportedExtensions({".mp3", ".flac", ".wav", ".m4a", ".ogg", ".aac"});

    auto scanCallback = [&logger](size_t count) { logger->info("[回调] 扫描完成, 共找到 {} 首歌曲.", count); };

    // 首次启动扫描
    logger->info("[主线程] 第一次请求 SongManager 开始扫描...");
    if (manager.startScan(scanCallback)) {
        logger->info("[主线程] 扫描任务已成功启动。");
    }

    // 在扫描进行中，立即尝试再次启动扫描
    logger->info("[主线程] 在扫描期间，立即再次请求扫描...");
    if (!manager.startScan(scanCallback)) {
        logger->warn("[主线程] 请求被拒绝，正如预期。因为已有任务在进行中。");
    }

    // 模拟主线程的事件循环
    logger->info("[主线程] 等待扫描结束...");
    while (manager.isScanning()) {
        // 安静等待，不再打印点
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    logger->info("[主线程] 检测到 isScanning() 返回 false，扫描已结束。");

    // 从 SongManager 获取所有歌曲并打印
    logger->info("[主线程] 从 SongManager 获取最终歌曲列表:");
    auto allSongs = manager.getAllSongs();
    if (allSongs.empty()) {
        logger->info("数据库为空或扫描的目标目录没有歌曲。");
    } else {
        logger->info("共获取到 {} 首歌曲，信息如下：", allSongs.size());
        for (const auto &song: allSongs) {
            printSongInfo(song, logger);
        }
    }

    // 演示搜索功能
    logger->info("演示搜索：搜索标题包含 '轻涟' 的歌曲:");
    auto searchResults = manager.searchSongs("轻涟");
    for (const auto &song: searchResults) {
        printSongInfo(song, logger);
    }

    // 调用 displaySongWithCover 显示第一首搜索结果的完整信息
    if (!searchResults.empty()) {
        logger->info("显示第一首搜索结果的封面及信息...");
        displaySongWithCover(searchResults[0]);
    } else {
        logger->info("未搜索到相关歌曲，无法显示封面。");
    }

     // === 演示导出功能 ===
    std::filesystem::path export_path = "../song_database_export.log";
    logger->info("[主线程] 正在尝试将数据库导出到 '{}'...", export_path.string());
    if (manager.exportDatabaseToFile(export_path)) {
        logger->info("[主线程] 数据库导出成功！");
    } else {
        logger->error("[主线程] 数据库导出失败。");
    }

    logger->info("--- 系统关闭 ---");

    // 打印获取到的歌曲名称
    auto songNames = manager.getSongNames();
    logger->info("扫描到的所有歌曲文件名:");
    for (const auto &songName: songNames) {
        logger->info("  - {}", songName);
    }
    return 0;
}
