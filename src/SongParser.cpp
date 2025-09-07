#include "SongParser.hpp" 

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

namespace SongParser {

    std::shared_ptr<spdlog::logger> logger;

    void logger_init() {
        if (!logger) {
            logger = spdlog::stdout_color_mt("SongParser");
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
        void getMetadataWithAPI(Song &song, AVFormatContext *formatCtx) {
            // 1. 获取时长
            if (formatCtx->duration != AV_NOPTS_VALUE) {
                song.duration = static_cast<int>(formatCtx->duration / AV_TIME_BASE);
            }

            // 2. 获取元数据标签
            const AVDictionary *metadata = formatCtx->metadata;
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                song.title = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "artist", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                song.artist = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "album", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                song.album = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "genre", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                song.genre = tag->value;
            }
            if (AVDictionaryEntry *tag = av_dict_get(metadata, "date", nullptr, AV_DICT_IGNORE_SUFFIX)) {
                try {
                    song.year = std::stoi(std::string(tag->value).substr(0, 4));
                } catch (...) {
                }
            }
        }

        // 提取封面图片
        void extractCoverArtWithAPI(Song &song, AVFormatContext *formatCtx) {
            int stream_idx = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

            if (stream_idx < 0) {
                logger->info("文件 '{}' 中未找到封面流。", song.filePath.string());
                return;
            }

            AVStream *stream = formatCtx->streams[stream_idx];

            // 检查流是否是附加图片 (Attached Picture)
            if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC && stream->attached_pic.size > 0) {
                const AVPacket &cover_packet = stream->attached_pic;
                song.coverArt.assign(cover_packet.data, cover_packet.data + cover_packet.size);

                // 尝试确定MIME类型
                AVCodecParameters *codec_params = stream->codecpar;
                if (codec_params->codec_id == AV_CODEC_ID_MJPEG || codec_params->codec_id == AV_CODEC_ID_JPEG2000) {
                    song.coverArtMimeType = "image/jpeg";
                } else if (codec_params->codec_id == AV_CODEC_ID_PNG) {
                    song.coverArtMimeType = "image/png";
                }
            }
        }

    } // namespace

    std::optional<Song> createSongFromFile(const std::filesystem::path &filePath) {
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
            return std::nullopt; // or return a partially filled song
        }

        Song song;
        song.filePath = filePath;

        // 调用新的API函数来填充Song结构体
        getMetadataWithAPI(song, formatCtx.get());
        extractCoverArtWithAPI(song, formatCtx.get());

        // formatCtx 会在函数结束时自动被unique_ptr的Deleter关闭和释放
        return song;
    }

} // namespace SongParser
