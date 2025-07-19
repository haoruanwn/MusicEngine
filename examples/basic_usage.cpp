// main.cpp
#include <chrono>
#include <iostream>
#include <thread>
#include "../include/SongManager.h"

void printSongInfo(const Song &song);

std::vector<std::filesystem::path> music_dirs = {
    "/home/hao/Projects/music_system/music_test",
    "/home/hao/Projects/music_system/music_test2"
};

int main() {

    std::cout << "--- 音乐播放器系统启动 ---" << std::endl;

    auto &manager = SongManager::getInstance();

    // manager.setDirectoryPath("/home/hao/Projects/music_system/music_test");
    manager.setDirectoryPath(music_dirs);

    auto scanCallback = [](size_t count) {
        std::cout << "\n[主线程回调] 扫描完成！共发现 " << count << " 首歌曲。" << std::endl;
    };

    // 3. 首次尝试启动扫描
    std::cout << "\n[主线程] 第一次请求 SongManager 开始扫描..." << std::endl;
    if (manager.startScan(scanCallback)) {
        std::cout << "[主线程] 扫描任务已成功启动。" << std::endl;
    }

    // 4. 在扫描进行中，立即尝试再次启动扫描
    std::cout << "\n[主线程] 在扫描期间，立即再次请求扫描..." << std::endl;
    if (!manager.startScan(scanCallback)) {
        std::cout << "[主线程] 请求被拒绝，正如预期。因为已有任务在进行中。" << std::endl;
    }

    // 5. 模拟主线程的事件循环
    std::cout << "\n[主线程] 进入模拟事件循环，等待扫描结束..." << std::endl;
    while (manager.isScanning()) {
        std::cout << ".";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "\n[主线程] 检测到 isScanning() 返回 false，扫描已结束。" << std::endl;

    // 6. 从 SongManager 获取所有歌曲并打印
    std::cout << "\n[主线程] 从 SongManager 获取最终歌曲列表:" << std::endl;
    auto allSongs = manager.getAllSongs();
    if (allSongs.empty()) {
        std::cout << "数据库为空或扫描的目标目录没有歌曲。" << std::endl;
    } else {
        std::cout << "共获取到 " << allSongs.size() << " 首歌曲，信息如下：" << std::endl;
        for (const auto &song: allSongs) {
            printSongInfo(song);
        }
    }

    // 7. 演示搜索功能
    std::cout << "\n[主线程] 演示搜索：搜索标题包含 '原色' 的歌曲:" << std::endl;
    auto searchResults = manager.searchSongs("原色");
    for (const auto &song: searchResults) {
        printSongInfo(song);
    }

    std::cout << "\n--- 系统关闭 ---" << std::endl;

    // 打印获取到的歌曲名称
    auto songNames = manager.getSongNames();
    std::cout << "歌曲名: " << std::endl;
    for (const auto &song: songNames) {
        std::cout << song << std::endl;
    }
    return 0;
}

// 用来打印歌曲信息
void printSongInfo(const Song &song) {
    std::cout << "===== 歌曲信息 =====" << std::endl;
    std::cout << "标题: " << (song.title.empty() ? "未知" : song.title) << std::endl;
    std::cout << "艺术家: " << (song.artist.empty() ? "未知" : song.artist) << std::endl;
    std::cout << "专辑: " << (song.album.empty() ? "未知" : song.album) << std::endl;
    std::cout << "文件路径: " << song.filePath << std::endl;
    std::cout << "封面大小: " << song.coverArt.size() << " 字节" << std::endl;
    std::cout << "封面 MIME 类型: " << (song.coverArtMimeType.empty() ? "未知" : song.coverArtMimeType) << std::endl;
    std::cout << "时长: " << song.duration << " 秒" << std::endl;
    std::cout << "年份: " << (song.year != 0 ? std::to_string(song.year) : "未知") << std::endl;
    std::cout << "====================" << std::endl;
}

