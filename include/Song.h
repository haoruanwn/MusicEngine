// song.h
#pragma once

#include <cstdint> // 为了使用 int32_t 等类型，更明确
#include <filesystem> // C++17: 引入文件系统库
#include <string>
#include <vector>

// 歌曲信息结构体
struct Song {
    // 基本信息
    std::string title; // 曲名
    std::string artist; // 艺术家
    std::string album; // 专辑
    std::string genre; // 流派
    int32_t year = 0; // 年份
    int32_t duration = 0; // 时长 (秒)

    std::filesystem::path filePath;

    // 专辑封面 (二进制数据)
    // 使用 vector<std::byte> (C++17) 或 vector<char> 来存储
    std::vector<char> coverArt;
    std::string coverArtMimeType; // 例如: "image/jpeg"
};
