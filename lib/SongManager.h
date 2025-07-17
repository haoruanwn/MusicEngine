// SongManager.h

#pragma once

#include "song.h"
#include <vector>
#include <string>
#include <filesystem>
#include <mutex>
#include <future>
#include <functional>
#include <atomic> // 新增：用于线程安全的布尔标志

class SongManager
{
public:
    static SongManager& getInstance();

    SongManager(const SongManager&) = delete;
    SongManager& operator=(const SongManager&) = delete;

    /**
     * @brief 异步扫描音乐目录.
     * 如果当前已有扫描任务正在进行，则此函数会立即返回false.
     * @param directoryPath 要扫描的目录路径.
     * @param onScanFinished (可选) 扫描完成时要调用的回调函数.
     * @return 如果成功启动扫描则返回true, 否则返回false.
     */
    // 修正1：函数签名与cpp文件匹配，使用 const& 提高效率
    bool startScan(const std::filesystem::path& directoryPath,
                   const std::function<void(size_t)>& onScanFinished = nullptr);

    /**
     * @brief 检查当前是否正在扫描.
     * @return 如果正在扫描则返回true.
     */
    bool isScanning() const;

    std::vector<Song> getAllSongs() const;
    std::vector<Song> searchSongs(const std::string& query) const;

private:
    SongManager(); // 构造函数移至cpp文件以配合原子变量初始化
    ~SongManager(); // 明确声明析构函数，以确保后台任务能被正确处理

    std::vector<Song> m_songDatabase;
    mutable std::mutex m_dbMutex;

    std::future<void> m_scanFuture;
    // 新增2：一个线程安全的标志，用于快速判断扫描状态
    std::atomic<bool> m_isScanning;
};