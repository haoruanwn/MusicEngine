#include "music_parser.hpp"

#include <cstdio>
#include <memory>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// Include FFmpeg library headers
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

namespace music_parser {

    std::shared_ptr<spdlog::logger> logger;

    void logger_init() {
        if (!logger) {
            logger = spdlog::stdout_color_mt("MusicParser");
            logger->set_level(spdlog::level::info);

            // Redirect FFmpeg logs to spdlog
            av_log_set_level(AV_LOG_WARNING);
            av_log_set_callback([](void *, int level, const char *fmt, va_list vl) {
                if (level > av_log_get_level())
                    return;
                char buffer[1024];
                vsnprintf(buffer, sizeof(buffer), fmt, vl);
                // Remove trailing newline
                buffer[strcspn(buffer, "\n")] = 0;
                logger->trace("FFmpeg: {}", buffer);
            });
        }
    }

    // Define a smart pointer deleter for AVFormatContext
    struct AVFormatContextDeleter {
        void operator()(AVFormatContext *ptr) const {
            if (ptr) {
                avformat_close_input(&ptr);
            }
        }
    };
    using AVFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;

    namespace {

        // Use FFmpeg API to get metadata
        void get_metadata_with_api(music_engine::Music &music, AVFormatContext *format_ctx) {
            // 1. Get duration
            if (format_ctx->duration != AV_NOPTS_VALUE) {
                music.duration = static_cast<int>(format_ctx->duration / AV_TIME_BASE);
            }

            // 2. Get metadata tags
            const AVDictionary *metadata = format_ctx->metadata;
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

        // Check if cover art exists
        void check_cover_art_exists(music_engine::Music &music, AVFormatContext *format_ctx) {
            int stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

            if (stream_index >= 0) {
                AVStream *stream = format_ctx->streams[stream_index];
                if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC && stream->attached_pic.size > 0) {
                    music.has_cover_art = true;
                }
            } else {
                music.has_cover_art = false;
            }
        }

    } // namespace

    std::optional<music_engine::Music> create_music_from_file(const std::filesystem::path &file_path) {
        // Use a smart pointer to manage the lifecycle of AVFormatContext
        AVFormatContext *format_ctx_raw = nullptr;
        // avformat_open_input allocates memory that we need to free manually; the RAII wrapper handles this automatically
        if (avformat_open_input(&format_ctx_raw, file_path.c_str(), nullptr, nullptr) != 0) {
            logger->warn("Cannot open file: {}", file_path.string());
            return std::nullopt;
        }
        AVFormatContextPtr format_ctx(format_ctx_raw); // Transfer ownership of the raw pointer to the smart pointer

        // Find stream information
        if (avformat_find_stream_info(format_ctx.get(), nullptr) < 0) {
            logger->warn("Cannot find stream information for file: {}", file_path.string());
            return std::nullopt; // or return a partially filled music
        }

        music_engine::Music music;
        music.file_path = file_path;

        // Call the new API functions to populate the Music struct
        get_metadata_with_api(music, format_ctx.get());
        check_cover_art_exists(music, format_ctx.get());

        // format_ctx will be automatically closed and released by the unique_ptr's Deleter at the end of the function
        return music;
    }

    std::optional<std::vector<char>> extract_cover_art_data(const std::filesystem::path &file_path) {
        AVFormatContext *format_ctx_raw = nullptr;
        if (avformat_open_input(&format_ctx_raw, file_path.c_str(), nullptr, nullptr) != 0) {
            logger->warn("extract_cover_art_data: Cannot open file: {}", file_path.string());
            return std::nullopt;
        }
        AVFormatContextPtr format_ctx(format_ctx_raw);

        if (avformat_find_stream_info(format_ctx.get(), nullptr) < 0) {
            logger->warn("extract_cover_art_data: Cannot find stream information for file: {}", file_path.string());
            return std::nullopt;
        }

        // Album art is usually stored as an attached video stream
        int stream_index = av_find_best_stream(format_ctx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (stream_index < 0) {
            logger->info("extract_cover_art_data: No video stream (cover art) found in file: {}", file_path.string());
            return std::nullopt;
        }

        AVStream *stream = format_ctx->streams[stream_index];

        // Check if this stream is an "attached picture" and contains data
        if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC && stream->attached_pic.size > 0) {
            const AVPacket &packet = stream->attached_pic;
            // Copy data from the packet to a vector
            return std::vector<char>(packet.data, packet.data + packet.size);
        }

        logger->info("extract_cover_art_data: Video stream is not an attached cover art: {}", file_path.string());
        return std::nullopt;
    }

} // namespace music_parser
