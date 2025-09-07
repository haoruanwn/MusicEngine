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

} // namespace MusicParser