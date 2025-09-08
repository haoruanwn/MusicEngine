#include "music_player.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// Include C library headers
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace MusicEngine {

    // Define the AudioFrame struct
    struct AudioFrame {
        std::vector<uint8_t> data;
        size_t consumed_bytes = 0; // Track the amount of data that has been consumed
    };

    // Pimpl (Pointer to implementation) struct, hiding all private members and complexity
    struct MusicPlayer::Impl {
        // --- Threading and Synchronization ---
        std::thread decoder_thread_;
        std::atomic<bool> stop_requested_{false};

        // --- State Management ---
        std::atomic<PlayerState> state_{PlayerState::Stopped};
        std::mutex control_mutex_;
        std::condition_variable control_cond_var_;

        // --- Data Buffer Queue ---
        std::queue<std::shared_ptr<AudioFrame>> frame_queue_;
        std::mutex queue_mutex_;
        std::condition_variable queue_cond_var_;
        static constexpr size_t MAX_QUEUE_SIZE = 50; // Buffer about 1 second of data

        // --- FFmpeg Related ---
        AVFormatContext *format_ctx_ = nullptr;
        AVCodecContext *codec_ctx_ = nullptr;
        SwrContext *swr_ctx_ = nullptr;
        int audio_stream_index_ = -1;

        // --- Audio Output ---
        ma_device audio_device_;
        ma_device_config device_config_;

        // --- Logging ---
        std::shared_ptr<spdlog::logger> logger_;

        // --- Playback Progress ---
        double total_duration_secs_{0.0};
        std::atomic<int64_t> total_samples_played_{0};
        std::atomic<double> seek_request_secs_{-1.0}; // -1.0 means no seek request

        Impl() {
            logger_ = spdlog::stdout_color_mt("MusicPlayer");
            logger_->set_level(spdlog::level::info);
        }

        // Member function declarations
        void decoder_loop();
        void process_playback_frames(void *p_output, ma_uint32 frame_count);
        void cleanup();

        static void audio_callback_wrapper(ma_device *p_device, void *p_output, const void *p_input,
                                           ma_uint32 frame_count) {
            Impl *p_impl = static_cast<Impl *>(p_device->pUserData);
            if (p_impl) {
                p_impl->process_playback_frames(p_output, frame_count);
            }
        }
    };

    MusicPlayer::MusicPlayer() : pimpl_(std::make_unique<Impl>()) {}

    MusicPlayer::~MusicPlayer() { stop(); }

    void MusicPlayer::play(const MusicEngine::Music &music) {
        stop(); // Before playing a new music, stop and clean up the old one

        pimpl_->stop_requested_ = false;

        // 重置样本计数器
        pimpl_->total_samples_played_ = 0;

        // 1. --- FFmpeg Initialization ---
        pimpl_->format_ctx_ = avformat_alloc_context();
        if (avformat_open_input(&pimpl_->format_ctx_, music.file_path.c_str(), nullptr, nullptr) != 0) {
            pimpl_->logger_->error("Cannot open file: {}", music.file_path.string());
            return;
        }
        if (avformat_find_stream_info(pimpl_->format_ctx_, nullptr) < 0) {
            pimpl_->logger_->error("Cannot find stream information for the file");
            pimpl_->cleanup();
            return;
        }

        // 计算并存储总时长
        pimpl_->total_duration_secs_ = static_cast<double>(pimpl_->format_ctx_->duration) / AV_TIME_BASE;

        // Find the best audio stream
        AVCodecParameters *codec_par = nullptr;
        pimpl_->audio_stream_index_ = av_find_best_stream(pimpl_->format_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        if (pimpl_->audio_stream_index_ < 0) {
            pimpl_->logger_->error("No audio stream found in the file");
            pimpl_->cleanup();
            return;
        }
        codec_par = pimpl_->format_ctx_->streams[pimpl_->audio_stream_index_]->codecpar;

        // Find the decoder
        const AVCodec *codec = avcodec_find_decoder(codec_par->codec_id);
        if (!codec) {
            pimpl_->logger_->error("Cannot find decoder");
            pimpl_->cleanup();
            return;
        }

        pimpl_->codec_ctx_ = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(pimpl_->codec_ctx_, codec_par) < 0) {
            pimpl_->logger_->error("Cannot copy decoder parameters");
            pimpl_->cleanup();
            return;
        }
        if (avcodec_open2(pimpl_->codec_ctx_, codec, nullptr) < 0) {
            pimpl_->logger_->error("Cannot open decoder");
            pimpl_->cleanup();
            return;
        }

        // 2. --- Audio Resampling (SwrContext) Initialization ---
        // We resample everything to a format that miniaudio handles easily: F32 stereo
        pimpl_->swr_ctx_ = swr_alloc();
        av_opt_set_chlayout(pimpl_->swr_ctx_, "in_chlayout", &pimpl_->codec_ctx_->ch_layout, 0);
        av_opt_set_int(pimpl_->swr_ctx_, "in_sample_rate", pimpl_->codec_ctx_->sample_rate, 0);
        av_opt_set_sample_fmt(pimpl_->swr_ctx_, "in_sample_fmt", pimpl_->codec_ctx_->sample_fmt, 0);

        AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
        av_opt_set_chlayout(pimpl_->swr_ctx_, "out_chlayout", &out_ch_layout, 0);
        av_opt_set_int(pimpl_->swr_ctx_, "out_sample_rate", pimpl_->codec_ctx_->sample_rate, 0);
        av_opt_set_sample_fmt(pimpl_->swr_ctx_, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
        swr_init(pimpl_->swr_ctx_);

        // 3. --- Miniaudio Initialization ---
        pimpl_->device_config_ = ma_device_config_init(ma_device_type_playback);
        pimpl_->device_config_.playback.format = ma_format_f32;
        pimpl_->device_config_.playback.channels = out_ch_layout.nb_channels;
        pimpl_->device_config_.sampleRate = pimpl_->codec_ctx_->sample_rate;
        pimpl_->device_config_.dataCallback = pimpl_->audio_callback_wrapper;
        pimpl_->device_config_.pUserData = pimpl_.get();

        if (ma_device_init(NULL, &pimpl_->device_config_, &pimpl_->audio_device_) != MA_SUCCESS) {
            pimpl_->logger_->error("Failed to initialize audio device");
            pimpl_->cleanup();
            return;
        }
        if (ma_device_start(&pimpl_->audio_device_) != MA_SUCCESS) {
            pimpl_->logger_->error("Failed to start audio device");
            pimpl_->cleanup();
            return;
        }

        // 所有初始化都成功了再设置播放状态
        pimpl_->state_ = PlayerState::Playing;

        // 4. --- Start the Decoder Thread ---
        pimpl_->decoder_thread_ = std::thread(&Impl::decoder_loop, pimpl_.get());
        pimpl_->logger_->info("Started playing: {}", music.file_path.string());
    }

    void MusicPlayer::stop() {
        if (pimpl_->state_ == PlayerState::Stopped) {
            return;
        }

        pimpl_->logger_->info("Stopping playback...");
        pimpl_->state_ = PlayerState::Stopped;
        pimpl_->stop_requested_ = true;

        // Wake up any waiting threads
        pimpl_->control_cond_var_.notify_one();
        pimpl_->queue_cond_var_.notify_all();

        if (pimpl_->decoder_thread_.joinable()) {
            pimpl_->decoder_thread_.join();
        }

        ma_device_uninit(&pimpl_->audio_device_);
        pimpl_->cleanup();

        // Clear the queue
        std::lock_guard<std::mutex> lock(pimpl_->queue_mutex_);
        std::queue<std::shared_ptr<AudioFrame>> empty_queue;
        pimpl_->frame_queue_.swap(empty_queue);
    }

    void MusicPlayer::pause() {
        // 检查状态
        if (pimpl_->state_ != PlayerState::Playing) {
            return;
        }

        pimpl_->state_ = PlayerState::Paused;

        // 显式停止 miniaudio 设备，这将停止调用我们的回调函数
        if (ma_device_stop(&pimpl_->audio_device_) != MA_SUCCESS) {
            pimpl_->logger_->warn("Failed to stop audio device on pause.");
        }

        pimpl_->logger_->info("Playback paused");
    }

    void MusicPlayer::resume() {
        // 检查状态必须放在前面
        if (pimpl_->state_ != PlayerState::Paused) {
            return;
        }

        pimpl_->state_ = PlayerState::Playing;

        // 显式启动 miniaudio 设备，这将恢复调用我们的回调函数
        if (ma_device_start(&pimpl_->audio_device_) != MA_SUCCESS) {
            pimpl_->logger_->error("Failed to start audio device on resume. Playback may not continue.");
            // 如果设备启动失败，最好还是停下来
            stop();
            return;
        }

        pimpl_->control_cond_var_.notify_one(); // 唤醒解码线程继续生产数据
        pimpl_->logger_->info("Playback resumed");
    }

    PlayerState MusicPlayer::get_state() const { return pimpl_->state_; }

    void MusicPlayer::Impl::cleanup() {
        avcodec_free_context(&codec_ctx_);
        avformat_close_input(&format_ctx_);
        swr_free(&swr_ctx_);
        audio_stream_index_ = -1;

        // 重置时长
        total_duration_secs_ = 0.0;
    }


    // ------------------- Producer-Consumer Core Logic -------------------

    // [Producer] Decoder Thread
    void MusicPlayer::Impl::decoder_loop() {
        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();

        while (!stop_requested_) {
            // 检查并处理 seek 请求
            double seek_pos = seek_request_secs_.exchange(-1.0);
            if (seek_pos >= 0.0) {
                logger_->info("Seek command received, processing...");
                // 计算FFmpeg时间戳
                int64_t target_timestamp =
                        av_rescale_q(static_cast<int64_t>(seek_pos * AV_TIME_BASE), {1, AV_TIME_BASE},
                                     format_ctx_->streams[audio_stream_index_]->time_base);

                // 执行 seek
                if (av_seek_frame(format_ctx_, audio_stream_index_, target_timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
                    logger_->error("Failed to seek to position {}", seek_pos);
                } else {
                    // 清空解码器缓冲区
                    avcodec_flush_buffers(codec_ctx_);

                    // 清空我们的队列
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex_);
                        std::queue<std::shared_ptr<AudioFrame>> empty_queue;
                        frame_queue_.swap(empty_queue);
                    }

                    // 更新播放样本计数器
                    total_samples_played_ = static_cast<int64_t>(seek_pos * device_config_.sampleRate);

                    logger_->info("Seek completed. Resuming decoding.");
                }
            }


            // Handle pause
            {
                std::unique_lock<std::mutex> lock(control_mutex_);
                control_cond_var_.wait(lock, [this] {
                    return state_ != PlayerState::Paused || stop_requested_ || seek_request_secs_ >= 0.0;
                });
            }
            if (stop_requested_)
                break;

            // Queue back-pressure control
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cond_var_.wait(lock, [this] {
                    return frame_queue_.size() < MAX_QUEUE_SIZE || stop_requested_ || seek_request_secs_ >= 0.0;
                });
            }
            if (stop_requested_)
                break;

            // 如果有新的 seek 请求，回到循环顶部处理
            if (seek_request_secs_ >= 0.0)
                continue;

            // Read and decode frames
            if (av_read_frame(format_ctx_, packet) >= 0) {
                if (packet->stream_index == audio_stream_index_) {
                    if (avcodec_send_packet(codec_ctx_, packet) == 0) {
                        while (avcodec_receive_frame(codec_ctx_, frame) == 0) {
                            auto audio_frame = std::make_shared<AudioFrame>();

                            // Resample
                            uint8_t **dst_data = nullptr;
                            int dst_linesize;
                            int dst_nb_samples =
                                    av_rescale_rnd(swr_get_delay(swr_ctx_, codec_ctx_->sample_rate) + frame->nb_samples,
                                                   codec_ctx_->sample_rate, codec_ctx_->sample_rate, AV_ROUND_UP);
                            av_samples_alloc_array_and_samples(&dst_data, &dst_linesize,
                                                               device_config_.playback.channels, dst_nb_samples,
                                                               AV_SAMPLE_FMT_FLT, 0);

                            int actual_dst_nb_samples = swr_convert(swr_ctx_, dst_data, dst_nb_samples,
                                                                    (const uint8_t **) frame->data, frame->nb_samples);

                            int data_size = actual_dst_nb_samples * device_config_.playback.channels *
                                            av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);
                            audio_frame->data.resize(data_size);
                            memcpy(audio_frame->data.data(), dst_data[0], data_size);

                            av_freep(&dst_data[0]);
                            av_freep(&dst_data);

                            // Push to queue
                            {
                                std::lock_guard<std::mutex> lock(queue_mutex_);
                                frame_queue_.push(audio_frame);
                            }
                            queue_cond_var_.notify_one();
                        }
                    }
                }
                av_packet_unref(packet);
            } else {
                // End of file
                stop_requested_ = true;
                logger_->info("Finished decoding file");
                break;
            }
        }

        av_packet_free(&packet);
        av_frame_free(&frame);
        logger_->info("Decoder thread exited");
    }

    // [Consumer] Audio Callback Processing
    void MusicPlayer::Impl::process_playback_frames(void *p_output, ma_uint32 frame_count) {
        uint8_t *p_output_u8 = static_cast<uint8_t *>(p_output);
        ma_uint32 frames_to_write = frame_count;
        ma_uint32 total_frames_written = 0;
        int bytes_per_frame = ma_get_bytes_per_frame(device_config_.playback.format, device_config_.playback.channels);

        while (frames_to_write > 0) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Wait for data in the queue, with a short timeout to prevent deadlocks if the decoder stalls
            if (queue_cond_var_.wait_for(lock, std::chrono::milliseconds(10),
                                         [this] { return !frame_queue_.empty() || stop_requested_; })) {
                if (stop_requested_) {
                    break;
                }

                auto &audio_frame = frame_queue_.front();
                size_t bytes_available = audio_frame->data.size() - audio_frame->consumed_bytes;
                size_t bytes_to_copy = std::min((size_t) frames_to_write * bytes_per_frame, bytes_available);

                memcpy(p_output_u8 + (total_frames_written * bytes_per_frame),
                       audio_frame->data.data() + audio_frame->consumed_bytes, bytes_to_copy);

                audio_frame->consumed_bytes += bytes_to_copy;
                ma_uint32 frames_copied = bytes_to_copy / bytes_per_frame;
                frames_to_write -= frames_copied;
                total_frames_written += frames_copied;

                if (audio_frame->consumed_bytes >= audio_frame->data.size()) {
                    frame_queue_.pop();
                    queue_cond_var_.notify_one();
                }

            } else {
                // Timeout, queue is empty, fill with silence
                break;
            }
        }

        // If there wasn't enough data, fill the rest with silence
        if (total_frames_written < frame_count) {
            ma_uint32 frames_to_silence = frame_count - total_frames_written;
            memset(p_output_u8 + (total_frames_written * bytes_per_frame), 0, frames_to_silence * bytes_per_frame);
        }

        // 累加实际写入的帧数到总播放样本数
        total_samples_played_ += total_frames_written;


        // If there wasn`t enough data, fill the rest with silence
        if (total_frames_written < frame_count) {
            // 静音填充
        }
    }

    double MusicPlayer::get_duration() const { return pimpl_->total_duration_secs_; }

    double MusicPlayer::get_current_position() const {
        if (pimpl_->audio_device_.sampleRate > 0) {
            return (double) pimpl_->total_samples_played_ / pimpl_->audio_device_.sampleRate;
        }
        return 0.0;
    }

    std::optional<double> MusicPlayer::seek(double position_secs) {
        if (pimpl_->state_ == PlayerState::Stopped) {
            pimpl_->logger_->warn("Seek request ignored: player is stopped.");
            return std::nullopt; // 请求被忽略
        }

        // 注意：这里我们放宽了对 position_secs 的检查，因为 seek_percent 依赖它
        // 具体的钳位应该由业务逻辑决定，或者在这里也加上
        if (position_secs < 0)
            position_secs = 0.0;
        if (position_secs > pimpl_->total_duration_secs_)
            position_secs = pimpl_->total_duration_secs_;


        pimpl_->logger_->info("Requesting seek to {} seconds", position_secs);
        pimpl_->seek_request_secs_ = position_secs;

        // 唤醒解码线程
        pimpl_->control_cond_var_.notify_one();
        pimpl_->queue_cond_var_.notify_one();

        return position_secs; // 返回实际请求的秒数
    }

    int MusicPlayer::get_current_position_percent() const {
        if (pimpl_->total_duration_secs_ <= 0) {
            return 0;
        }
        double current_position = get_current_position();
        double percentage = (current_position / pimpl_->total_duration_secs_) * 100.0;
        return static_cast<int>(std::round(percentage)); // 四舍五入取整
    }

    std::optional<int> MusicPlayer::seek_percent(int percentage) {
        if (pimpl_->state_ == PlayerState::Stopped) {
            pimpl_->logger_->warn("Seek percentage request ignored: player is stopped.");
            return std::nullopt; // 请求被忽略
        }

        int clamped_percentage = std::max(0, std::min(100, percentage));
        if (clamped_percentage != percentage) {
            pimpl_->logger_->warn("Seek percentage {} is out of range. Clamped to {}.", percentage, clamped_percentage);
        }

        double target_secs = pimpl_->total_duration_secs_ * (static_cast<double>(clamped_percentage) / 100.0);

        // 复用已有的 seek(double) 函数
        seek(target_secs);

        return clamped_percentage; // 返回钳位后的百分比
    }

} // namespace MusicEngine
