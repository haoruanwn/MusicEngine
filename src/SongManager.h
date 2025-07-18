// SongManager.h

#pragma once

#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <vector>
#include "Song.h"

class SongManager {
public:
    static SongManager &getInstance();

    SongManager(const SongManager &) = delete;
    SongManager &operator=(const SongManager &) = delete;

    /**
     * @brief 异步扫描音乐目录.
     * 如果当前已有扫描任务正在进行，则此函数会立即返回false.
     * @param directoryPath 要扫描的目录路径.
     * @param onScanFinished (可选) 扫描完成时要调用的回调函数.
     * @return 如果成功启动扫描则返回true, 否则返回false.
     */
    bool startScan(const std::function<void(size_t)> &onScanFinished = nullptr);

    /**
     * @brief 检查当前是否正在扫描.
     * @return 如果正在扫描则返回true.
     */
    bool isScanning() const;

    std::vector<Song> getAllSongs() const;
    std::vector<Song> searchSongs(const std::string &query) const;

    // 用以获取目前扫描到的歌曲名
    std::vector<std::string> getSongNames() const;

    void setDirectoryPath(const std::filesystem::path &directoryPath);

private:
    SongManager();
    ~SongManager();

    std::vector<Song> m_songDatabase;
    mutable std::mutex m_dbMutex;

    std::future<void> m_scanFuture;

    std::atomic<bool> m_isScanning;

    std::filesystem::path m_directoryPath;
};
