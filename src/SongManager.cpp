// SongManager.cpp

#include "../include/SongManager.h"
#include <algorithm>
#include <iostream>
#include "internal/SongParser.h"

// 将构造函数实现移到cpp文件
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
    // 改进1：检查是否已有扫描任务正在进行
    if (m_isScanning) {
        std::cout << "[SongManager] 错误: 上一个扫描任务仍在进行中。" << std::endl;
        return false;
    }

    m_isScanning = true; // 设置扫描标志

    m_scanFuture = std::async(std::launch::async, [this, onScanFinished]() {
        std::cout << "[SongManager] 后台扫描开始于目录: " << m_directoryPath << std::endl;

        std::vector<Song> newDatabase;
        const std::vector<std::string> supportedExtensions = {".mp3", ".m4a", ".flac"};

        // 使用 try-catch 块来处理文件系统可能抛出的异常 (如权限问题)
        try {
            for (const auto &entry: std::filesystem::recursive_directory_iterator(m_directoryPath)) {
                if (entry.is_regular_file()) {
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

        size_t count = newDatabase.size();
        std::cout << "[SongManager] 扫描完成, 共找到 " << count << " 首歌曲." << std::endl;

        {
            std::lock_guard<std::mutex> lock(m_dbMutex);
            m_songDatabase = std::move(newDatabase);
        }

        if (onScanFinished) {
            onScanFinished(count);
        }

        m_isScanning = false; // 扫描结束，重置标志
    });

    return true;
}

bool SongManager::isScanning() const { return m_isScanning; }

std::vector<Song> SongManager::getAllSongs() const {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    return m_songDatabase;
}

std::vector<Song> SongManager::searchSongs(const std::string &query) const {
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

    if (m_directoryPath.empty())
        std::cerr << "[SongManager] 错误: 目录路径未设置。" << std::endl;

    for (const auto &song: m_songDatabase) {
        std::filesystem::path relativePath = std::filesystem::relative(song.filePath, m_directoryPath);
        results.push_back(relativePath.string());
    }
    return results;
}

void SongManager::setDirectoryPath(const std::filesystem::path &directoryPath) {
    m_directoryPath = directoryPath;
}

