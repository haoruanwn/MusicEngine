#pragma once

#include <filesystem>
#include <optional>
#include "Music.h" // 包含Music结构体的定义

using namespace MusicEngine;

namespace MusicParser {

    void logger_init();

    /**
     * @brief 从音频文件中解析元数据和封面。
     * @param filePath 要解析的音乐文件的路径。
     * @return 如果成功，则返回一个包含音乐信息的 std::optional<Music>；否则返回 std::nullopt。
     */
    std::optional<Music> createMusicFromFile(const std::filesystem::path &filePath);

    /**
     * @brief 从文件中提取封面数据。
     * @param filePath 音乐文件路径。
     * @return 包含封面数据的vector，如果失败或不存在则返回std::nullopt。
     */
    std::optional<std::vector<char>> extractCoverArtData(const std::filesystem::path &filePath);



} // namespace MusicParser