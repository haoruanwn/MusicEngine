#pragma once


#include <filesystem>
#include <fstream> // 用于文件读取
#include <optional>
#include <string>
#include <vector>

#include "Song.h"
#include "ffprobe.hpp"
#include "ParserLogger.hpp"

namespace SongParser {

    /**
     * @brief 使用 ffmpeg 从文件中提取封面
     * @param song 一个已经填充了文件路径的Song对象
     */
    inline void extractCoverArtWithFFmpeg(Song &song) {
        std::filesystem::path tempCoverPath = std::filesystem::temp_directory_path() / (song.filePath.stem().string() + "_cover.jpg");
        
        std::string escapedInputPath = escapeSingleQuotes(song.filePath.string());
        std::string escapedOutputPath = escapeSingleQuotes(tempCoverPath.string());

        std::string command = "ffmpeg -y -v error -i " + escapedInputPath + " -an -c:v copy -frames:v 1 " + escapedOutputPath;

        try {
            executeCommand(command.c_str());
            if (std::filesystem::exists(tempCoverPath) && !std::filesystem::is_empty(tempCoverPath)) {
                
                std::ifstream file(tempCoverPath, std::ios::binary);
                if (file) {
                    song.coverArt = std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    song.coverArtMimeType = "image/jpeg"; // ffprobe 告诉我们是mjpeg
                }
                
                std::filesystem::remove(tempCoverPath);
            }
        } catch (const std::exception& e) {
            ParserLog::logger->error("使用ffmpeg提取封面时发生错误: {}。文件: {}", e.what(), song.filePath.string());
        }
    }


    /**
     * @brief 从音频文件中解析元数据和封面.
     * 此函数完全依赖 ffprobe 和 ffmpeg
     */
    inline std::optional<Song> createSongFromFile(const std::filesystem::path &filePath) {
        
        // 调用 ffprobe 安全地获取所有文本元数据
        auto songOpt = getMetadataWithFFprobe(filePath);

        if (!songOpt) {
            return std::nullopt;
        }
        
        Song song = *songOpt;

        // 调用 ffmpeg 安全地提取封面
        extractCoverArtWithFFmpeg(song);

        return song;
    }
} // namespace SongParser