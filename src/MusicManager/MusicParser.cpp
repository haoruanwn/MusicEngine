#include "MusicParser.hpp"

#include <cstdio>
#include <memory>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// 引入 FFmpeg 库的头文件
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

namespace MusicParser {

    std::shared_ptr<spdlog::logger> logger;

    void logger_init() {
        if (!logger) {
            logger = spdlog::stdout_color_mt("MusicParser");
            logger->set_level(spdlog::level::info);

            // 将FFmpeg的日志重定向到spdlog
            av_log_set_level(AV_LOG_WARNING);
            av_log_set_callback([](void *, int level, const char *fmt, va_list vl) {
                if (level > av_log_get_level())
                    return;
                char buffer[1024];
                vsnprintf(buffer, sizeof(buffer), fmt, vl);
                // 去掉末尾的换行符
                buffer[strcspn(buffer, "\n")] = 0;
                logger->trace("FFmpeg: {}", buffer);
            });
        }
    }

    // 为 AVFormatContext 定义一个智能指针删除器
    struct AVFormatContextDeleter {
        void operator()(AVFormatContext *ptr) const {
            if (ptr) {
                avformat_close_input(&ptr);
            }
        }
    };
    using AVFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;

    namespace {

        // 使用 FFmpeg API 获取元数据
        void getMetadataWithAPI(Music &music, AVFormatContext *formatCtx) {
            // 1. 获取时长
            if (formatCtx->duration != AV_NOPTS_VALUE) {
                music.duration = static_cast<int>(formatCtx->duration / AV_TIME_BASE);
            }

            // 2. 获取元数据标签
            const AVDictionary *metadata = formatCtx->metadata;
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                music.title = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "artist", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                music.artist = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "album", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                music.album = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "genre", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                music.genre = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "date", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                try {
                    music.year = std::stoi(std::string(tag->value).substr(0, 4));
                } catch (...) {
                }
            }
        }

        // 检查封面是否存在
        void checkCoverArtExists(Music &music, AVFormatContext *formatCtx) {
            int stream_idx = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

            if (stream_idx >= 0) {
                AVStream *stream = formatCtx->streams[stream_idx];
                if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC && stream->attached_pic.size > 0) {
                    music.hasCoverArt = true;
                }
            } else {
                music.hasCoverArt = false;
            }
        }

    } // namespace

    std::optional<Music> createMusicFromFile(const std::filesystem::path &filePath) {
        // 使用智能指针管理 AVFormatContext 的生命周期
        AVFormatContext *formatCtxRaw = nullptr;
        // avformat_open_input 会分配内存，需要我们手动释放，RAII包装器会自动处理
        if (avformat_open_input(&formatCtxRaw, filePath.c_str(), nullptr, nullptr) != 0) {
            logger->warn("无法打开文件: {}", filePath.string());
            return std::nullopt;
        }
        AVFormatContextPtr formatCtx(formatCtxRaw); // 将裸指针交给智能指针管理

        // 查找流信息
        if (avformat_find_stream_info(formatCtx.get(), nullptr) < 0) {
            logger->warn("无法找到文件流信息: {}", filePath.string());
            return std::nullopt; // or return a partially filled music
        }

        Music music;
        music.filePath = filePath;

        // 调用新的API函数来填充Music结构体
        getMetadataWithAPI(music, formatCtx.get());
        checkCoverArtExists(music, formatCtx.get());

        // formatCtx 会在函数结束时自动被unique_ptr的Deleter关闭和释放
        return music;
    }

    std::optional<std::vector<char>> extractCoverArtData(const std::filesystem::path &filePath) {
        AVFormatContext *formatCtxRaw = nullptr;
        if (avformat_open_input(&formatCtxRaw, filePath.c_str(), nullptr, nullptr) != 0) {
            logger->warn("extractCoverArtData: 无法打开文件: {}", filePath.string());
            return std::nullopt;
        }
        AVFormatContextPtr formatCtx(formatCtxRaw);

        if (avformat_find_stream_info(formatCtx.get(), nullptr) < 0) {
            logger->warn("extractCoverArtData: 无法找到文件流信息: {}", filePath.string());
            return std::nullopt;
        }

        // 专辑封面通常作为附加的视频流存储
        int stream_idx = av_find_best_stream(formatCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (stream_idx < 0) {
            logger->info("extractCoverArtData: 文件中未找到视频流 (封面): {}", filePath.string());
            return std::nullopt;
        }

        AVStream *stream = formatCtx->streams[stream_idx];

        // 检查该流是否是“附加图片”并且包含数据
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC && stream->attached_pic.size > 0) {
            const AVPacket &packet = stream->attached_pic;
            // 从 packet 中拷贝数据到 vector
            return std::vector<char>(packet.data, packet.data + packet.size);
        }

        logger->info("extractCoverArtData: 视频流不是附加封面: {}", filePath.string());
        return std::nullopt;
    }

} // namespace MusicParser
