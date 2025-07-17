// SongParser.h

#pragma once

#include <filesystem>
#include <optional>
#include "song.h" // 依赖 Song 结构体定义

// 我们将所有解析相关的函数都放在 SongParser 命名空间下
// 这是一个好习惯，可以避免函数名与项目的其他部分冲突
namespace SongParser {

    /**
     * @brief 从一个音频文件中解析元数据和封面.
     * * 使用 TagLib 库来读取音频文件的标签信息。
     * 目前对提取 MP3 (ID3v2) 格式的封面有特别支持。
     *
     * @param filePath 指向音频文件的 std::filesystem::path 对象.
     * @return 如果解析成功, 返回一个包含 Song 数据的 std::optional;
     * 如果文件无效或无法读取标签, 则返回 std::nullopt.
     */
    std::optional<Song> createSongFromFile(const std::filesystem::path &filePath);

} // namespace SongParser
