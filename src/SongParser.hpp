#pragma once

#include <filesystem>
#include <optional>
#include "Song.h" // 包含Song结构体的定义

namespace SongParser {

    void logger_init();

    /**
     * @brief 从音频文件中解析元数据和封面。
     * 此函数是该模块的主要入口点，完全依赖 ffprobe 和 ffmpeg。
     * @param filePath 要解析的音乐文件的路径。
     * @return 如果成功，则返回一个包含歌曲信息的 std::optional<Song>；否则返回 std::nullopt。
     */
    std::optional<Song> createSongFromFile(const std::filesystem::path &filePath);

} // namespace SongParser