#pragma once

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include "Song.h"
#include "ParserLogger.hpp"

#include "jsoncons/json.hpp"
#include "jsoncons/basic_json.hpp"

// 调用ffprobe获取音频文件的元数据


// 一个通用的执行命令并获取输出的函数
inline std::string executeCommand(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

namespace SongParser {
    // Helper function to escape filenames for shell command
    inline std::string escapeSingleQuotes(const std::string &s) {
        std::string escaped;
        for (char c: s) {
            if (c == '\'') {
                escaped += "'\\''"; // a'b -> 'a'\''b'
            } else {
                escaped += c;
            }
        }
        return "'" + escaped + "'";
    }

    /**
     * @brief 使用 ffprobe 安全地解析文件的元数据 (除封面外)
     * @param filePath 音乐文件的路径
     * @return 包含元数据的 std::optional<Song>, 如果失败则返回 std::nullopt
     */
    inline std::optional<Song> getMetadataWithFFprobe(const std::filesystem::path &filePath) {
        std::string escapedPath = escapeSingleQuotes(filePath.string());
        std::string command = "ffprobe -v quiet -print_format json -show_format " + escapedPath;

        Song song;
        song.filePath = filePath;

        try {
            std::string jsonOutput = executeCommand(command.c_str());
            if (jsonOutput.empty() || jsonOutput.length() < 10) { // 增加一个更稳妥的长度检查
                return song;
            }

            jsoncons::json doc = jsoncons::json::parse(jsonOutput);

            if (doc.contains("format")) {
                const auto &format = doc["format"];
                if (format.contains("duration")) {
                    song.duration = static_cast<int>(std::stod(format["duration"].as_string()));
                }

                if (format.contains("tags")) {
                    const auto &tags = format["tags"];

                    // --- 核心修正：匹配JSON中的大小写 ---
                    if (tags.contains("Title"))
                        song.title = tags["Title"].as_string();
                    if (tags.contains("Artist"))
                        song.artist = tags["Artist"].as_string();
                    if (tags.contains("Album"))
                        song.album = tags["Album"].as_string();
                    if (tags.contains("Genre"))
                        song.genre = tags["Genre"].as_string(); // ffprobe也可能输出大写的Genre

                    // 年份的逻辑保持不变，因为它已经很健壮
                    if (tags.contains("date")) {
                        try {
                            song.year = std::stoi(tags["date"].as_string().substr(0, 4));
                        } catch (...) { /* 忽略年份解析错误 */
                        }
                    } else if (tags.contains("TYER")) {
                        try {
                            song.year = std::stoi(tags["TYER"].as_string());
                        } catch (...) {
                        }
                    }
                }
            }
        } catch (const std::exception &e) {
            ParserLog::logger->warn("调用ffprobe或解析其输出时出错: {} 文件: {}", e.what(), filePath.string());
            return song;
        }
        return song;
    }
} // namespace SongParser
