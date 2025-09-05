#include "SongParser.hpp" // 首先包含我们自己的头文件

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>

#include "jsoncons/json.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


namespace SongParser {

    std::shared_ptr<spdlog::logger> logger;

    void logger_init() {
        if (!logger) {
            logger = spdlog::stdout_color_mt("SongParser");
            logger->set_level(spdlog::level::info);
        }
    }

    namespace {
        std::string executeCommand(const char *cmd) {
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                logger->critical("popen() failed for command: {}", cmd);
                return "";
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            return result;
        }

        std::string escapeSingleQuotes(const std::string &s) {
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

        std::optional<Song> getMetadataWithFFprobe(const std::filesystem::path &filePath) {
            std::string escapedPath = escapeSingleQuotes(filePath.string());
            std::string command = "ffprobe -v quiet -print_format json -show_format " + escapedPath;

            Song song;
            song.filePath = filePath;

            try {
                std::string jsonOutput = executeCommand(command.c_str());
                if (jsonOutput.empty() || jsonOutput.length() < 10) {
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
                        if (tags.contains("Title"))
                            song.title = tags["Title"].as_string();
                        if (tags.contains("Artist"))
                            song.artist = tags["Artist"].as_string();
                        if (tags.contains("Album"))
                            song.album = tags["Album"].as_string();
                        if (tags.contains("Genre"))
                            song.genre = tags["Genre"].as_string();
                        if (tags.contains("date")) {
                            try {
                                song.year = std::stoi(tags["date"].as_string().substr(0, 4));
                            } catch (...) {
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
                logger->warn("调用ffprobe或解析其输出时出错: {} 文件: {}", e.what(), filePath.string());
                return song;
            }
            return song;
        }

        void extractCoverArtWithFFmpeg(Song &song) {
            std::filesystem::path tempCoverPath =
                    std::filesystem::temp_directory_path() / (song.filePath.stem().string() + "_cover.jpg");
            std::string escapedInputPath = escapeSingleQuotes(song.filePath.string());
            std::string escapedOutputPath = escapeSingleQuotes(tempCoverPath.string());
            std::string command =
                    "ffmpeg -y -v error -i " + escapedInputPath + " -an -c:v copy -frames:v 1 " + escapedOutputPath;
            try {
                executeCommand(command.c_str());
                if (std::filesystem::exists(tempCoverPath) && !std::filesystem::is_empty(tempCoverPath)) {
                    std::ifstream file(tempCoverPath, std::ios::binary);
                    if (file) {
                        song.coverArt = std::vector<char>((std::istreambuf_iterator<char>(file)),
                                                          std::istreambuf_iterator<char>());
                        song.coverArtMimeType = "image/jpeg";
                    }
                    std::filesystem::remove(tempCoverPath);
                }
            } catch (const std::exception &e) {
                logger->error("使用ffmpeg提取封面时发生错误: {}。文件: {}", e.what(), song.filePath.string());
            }
        }

    } // namespace

    std::optional<Song> createSongFromFile(const std::filesystem::path &filePath) {
        auto songOpt = getMetadataWithFFprobe(filePath);
        if (!songOpt) {
            return std::nullopt;
        }
        Song song = *songOpt;
        extractCoverArtWithFFmpeg(song);
        return song;
    }

} // namespace SongParser
