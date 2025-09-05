// SongManager.cpp

#include "../include/SongManager.h"
#include <algorithm>
#include <iostream>
#include "SongParser.h"

SongManager::SongManager() : m_isScanning(false) {}

// 实现析构函数
SongManager::~SongManager() {
    // 如果程序退出时，后台扫描仍在进行，则等待其完成
    if (m_scanFuture.valid()) {
        m_scanFuture.wait();
    }
}

SongManager &SongManager::getInstance() {
    static SongManager instance;
    return instance;
}

bool SongManager::startScan(const std::function<void(size_t)> &onScanFinished) {
    // 检查目录路径是否已设置
    if (m_directoryPaths.empty()) {
        std::cerr << "[SongManager] 错误: 目录路径未设置。" << std::endl;
        return false;
    }

    // 检查是否已有扫描任务正在进行
    if (m_isScanning) {
        std::cout << "[SongManager] 错误: 上一个扫描任务仍在进行中。" << std::endl;
        return false;
    }

    m_isScanning = true; // 设置扫描标志

    // 复制一份路径，确保在异步任务中使用的路径不会被修改
    auto pathsToScan = m_directoryPaths;

    // 调用std::async启用一个异步任务（传入一个lambda表达式）
    m_scanFuture = std::async(std::launch::async, [this, pathsToScan, onScanFinished]() {
        std::cout << "[SongManager] 后台扫描开始..." << std::endl;

        std::vector<Song> newDatabase;
        const std::vector<std::string> supportedExtensions = {".mp3", ".m4a", ".flac"};

        for (const auto &dirPath: pathsToScan) {
            std::cout << "[SongManager] ==> 正在扫描: " << dirPath << std::endl;
            try {
                // recursive_directory_iterator 必须在循环内部对单个路径使用
                for (const auto &entry: std::filesystem::recursive_directory_iterator(dirPath)) {
                    if (entry.is_regular_file()) {

                        // 打印出目前正在处理的文件名
                        std::cout << "[SongManager] 正在处理: " << entry.path().string() << std::endl;
                        
                        std::string extension = entry.path().extension().string();
                        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                        for (const auto &supExt: supportedExtensions) {
                            if (extension == supExt) {
                                if (auto songOpt = SongParser::createSongFromFile(entry.path())) {
                                    newDatabase.push_back(*songOpt);
                                }
                                break;
                            }
                        }
                    }
                }
            } catch (const std::filesystem::filesystem_error &e) {
                std::cerr << "[SongManager] 文件系统错误: " << e.what() << std::endl;
            }
        }

        size_t count = newDatabase.size();
        std::cout << "[SongManager] 扫描完成, 共找到 " << count << " 首歌曲." << std::endl;

        {
            std::lock_guard<std::mutex> lock(m_dbMutex);
            m_songDatabase = std::move(newDatabase);
        }

        if (onScanFinished) {
            onScanFinished(count);
        }

        m_isScanning = false;
    });

    return true;
}

bool SongManager::isScanning() const { return m_isScanning; }

std::vector<Song> SongManager::getAllSongs() const {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    return m_songDatabase;
}

std::vector<Song> SongManager::searchSongs(const std::string &query) const {
    // 检查目录路径是否已设置
    if (m_directoryPaths.empty())
        std::cerr << "[SongManager] 错误: 目录路径未设置。" << std::endl;

    std::vector<Song> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    std::lock_guard<std::mutex> lock(m_dbMutex);
    for (const auto &song: m_songDatabase) {
        std::string lowerTitle = song.title;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);

        if (lowerTitle.find(lowerQuery) != std::string::npos) {
            results.push_back(song);
        }
    }
    return results;
}

std::vector<std::string> SongManager::getSongNames() const {
    std::vector<std::string> results;

    if (m_directoryPaths.empty())
        std::cerr << "[SongManager] 错误: 目录路径未设置。" << std::endl;

    for (const auto &song: m_songDatabase) {
        results.push_back(song.filePath.filename().string());
    }
    return results;
}

// setDirectoryPath 方法设定中只能执行一次，所以每次执行的时候清理之前的
void SongManager::setDirectoryPath(const std::vector<std::filesystem::path> &directoryPaths) {
    // 清空容器
    m_directoryPaths.clear();
    m_directoryPaths = directoryPaths;
}

void SongManager::setDirectoryPath(const std::filesystem::path &directoryPath) {
    // 清空容器
    m_directoryPaths.clear();
    m_directoryPaths.push_back(directoryPath);
}
