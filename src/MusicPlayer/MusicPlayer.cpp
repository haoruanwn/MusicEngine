#include "MusicPlayer.h"

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

// 引入 C 语言库头文件
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace MusicEngine {

    // 定义音频帧结构体
    struct AudioFrame {
        std::vector<uint8_t> data;
        size_t consumed_bytes = 0; // 记录已被消费的数据量
    };

    // Pimpl (Pointer to implementation) 结构体，隐藏所有私有成员和复杂性
    struct MusicPlayer::Impl {
        // --- 线程与同步 ---
        std::thread m_decoderThread;
        std::atomic<bool> m_stopRequested{false};

        // --- 状态管理 ---
        std::atomic<PlayerState> m_state{PlayerState::Stopped};
        std::mutex m_controlMutex;
        std::condition_variable m_controlCondVar;

        // --- 数据缓冲队列 ---
        std::queue<std::shared_ptr<AudioFrame>> m_frameQueue;
        std::mutex m_queueMutex;
        std::condition_variable m_queueCondVar;
        static constexpr size_t MAX_QUEUE_SIZE = 50; // 缓冲约1秒的数据

        // --- FFmpeg 相关 ---
        AVFormatContext *m_formatCtx = nullptr;
        AVCodecContext *m_codecCtx = nullptr;
        SwrContext *m_swrCtx = nullptr;
        int m_audioStreamIndex = -1;

        // --- 音频输出 ---
        ma_device m_audioDevice;
        ma_device_config m_deviceConfig;

        // --- 日志 ---
        std::shared_ptr<spdlog::logger> m_logger;

        Impl() {
            m_logger = spdlog::stdout_color_mt("MusicPlayer");
            m_logger->set_level(spdlog::level::info);
        }

        // 成员函数声明
        void decoderLoop();
        void processPlaybackFrames(void *pOutput, ma_uint32 frameCount);
        void cleanup();

        static void audio_callback_wrapper(ma_device *pDevice, void *pOutput, const void *pInput,
                                           ma_uint32 frameCount) {
            MusicPlayer::Impl *pImpl = static_cast<MusicPlayer::Impl *>(pDevice->pUserData);
            if (pImpl) {
                pImpl->processPlaybackFrames(pOutput, frameCount);
            }
        }
    };

    MusicPlayer::MusicPlayer() : pimpl(std::make_unique<Impl>()) {}

    MusicPlayer::~MusicPlayer() { stop(); }

    void MusicPlayer::play(const Music &music) {
        stop(); // 播放新歌曲前，先停止并清理旧的

        pimpl->m_stopRequested = false;
        pimpl->m_state = PlayerState::Playing;

        // 1. --- FFmpeg 初始化 ---
        pimpl->m_formatCtx = avformat_alloc_context();
        if (avformat_open_input(&pimpl->m_formatCtx, music.filePath.c_str(), nullptr, nullptr) != 0) {
            pimpl->m_logger->error("无法打开文件: {}", music.filePath.string());
            return;
        }
        if (avformat_find_stream_info(pimpl->m_formatCtx, nullptr) < 0) {
            pimpl->m_logger->error("无法找到文件流信息");
            pimpl->cleanup();
            return;
        }

        // 寻找最佳音频流
        AVCodecParameters *codecPar = nullptr;
        pimpl->m_audioStreamIndex = av_find_best_stream(pimpl->m_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        if (pimpl->m_audioStreamIndex < 0) {
            pimpl->m_logger->error("文件中未找到音频流");
            pimpl->cleanup();
            return;
        }
        codecPar = pimpl->m_formatCtx->streams[pimpl->m_audioStreamIndex]->codecpar;

        // 寻找解码器
        const AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);
        if (!codec) {
            pimpl->m_logger->error("找不到解码器");
            pimpl->cleanup();
            return;
        }

        pimpl->m_codecCtx = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(pimpl->m_codecCtx, codecPar) < 0) {
            pimpl->m_logger->error("无法复制解码器参数");
            pimpl->cleanup();
            return;
        }
        if (avcodec_open2(pimpl->m_codecCtx, codec, nullptr) < 0) {
            pimpl->m_logger->error("无法打开解码器");
            pimpl->cleanup();
            return;
        }

        // 2. --- 音频重采样 (SwrContext) 初始化 ---
        // 无论源格式如何，我们都统一重采样为 miniaudio 容易处理的 F32 立体声格式
        pimpl->m_swrCtx = swr_alloc();
        av_opt_set_chlayout(pimpl->m_swrCtx, "in_chlayout", &pimpl->m_codecCtx->ch_layout, 0);
        av_opt_set_int(pimpl->m_swrCtx, "in_sample_rate", pimpl->m_codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(pimpl->m_swrCtx, "in_sample_fmt", pimpl->m_codecCtx->sample_fmt, 0);

        AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
        av_opt_set_chlayout(pimpl->m_swrCtx, "out_chlayout", &out_ch_layout, 0);
        av_opt_set_int(pimpl->m_swrCtx, "out_sample_rate", pimpl->m_codecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(pimpl->m_swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
        swr_init(pimpl->m_swrCtx);

        // 3. --- Miniaudio 初始化 ---
        pimpl->m_deviceConfig = ma_device_config_init(ma_device_type_playback);
        pimpl->m_deviceConfig.playback.format = ma_format_f32;
        pimpl->m_deviceConfig.playback.channels = out_ch_layout.nb_channels;
        pimpl->m_deviceConfig.sampleRate = pimpl->m_codecCtx->sample_rate;
        pimpl->m_deviceConfig.dataCallback = pimpl->audio_callback_wrapper;
        pimpl->m_deviceConfig.pUserData = pimpl.get();

        if (ma_device_init(NULL, &pimpl->m_deviceConfig, &pimpl->m_audioDevice) != MA_SUCCESS) {
            pimpl->m_logger->error("初始化音频设备失败");
            pimpl->cleanup();
            return;
        }
        if (ma_device_start(&pimpl->m_audioDevice) != MA_SUCCESS) {
            pimpl->m_logger->error("启动音频设备失败");
            pimpl->cleanup();
            return;
        }

        // 4. --- 启动解码线程 ---
        pimpl->m_decoderThread = std::thread(&Impl::decoderLoop, pimpl.get());
        pimpl->m_logger->info("开始播放: {}", music.filePath.string());
    }

    void MusicPlayer::stop() {
        if (pimpl->m_state == PlayerState::Stopped) {
            return;
        }

        pimpl->m_logger->info("正在停止播放...");
        pimpl->m_state = PlayerState::Stopped;
        pimpl->m_stopRequested = true;

        // 唤醒可能在等待的线程
        pimpl->m_controlCondVar.notify_one();
        pimpl->m_queueCondVar.notify_all();

        if (pimpl->m_decoderThread.joinable()) {
            pimpl->m_decoderThread.join();
        }

        ma_device_uninit(&pimpl->m_audioDevice);
        pimpl->cleanup();

        // 清空队列
        std::lock_guard<std::mutex> lock(pimpl->m_queueMutex);
        std::queue<std::shared_ptr<AudioFrame>> emptyQueue;
        pimpl->m_frameQueue.swap(emptyQueue);
    }

    void MusicPlayer::pause() {
        if (pimpl->m_state == PlayerState::Playing) {
            pimpl->m_state = PlayerState::Paused;
            pimpl->m_logger->info("播放已暂停");
        }
    }

    void MusicPlayer::resume() {
        if (pimpl->m_state == PlayerState::Paused) {
            pimpl->m_state = PlayerState::Playing;
            pimpl->m_controlCondVar.notify_one(); // 唤醒解码线程
            pimpl->m_logger->info("播放已恢复");
        }
    }

    PlayerState MusicPlayer::getState() const { return pimpl->m_state; }

    void MusicPlayer::Impl::cleanup() {
        avcodec_free_context(&m_codecCtx);
        avformat_close_input(&m_formatCtx);
        swr_free(&m_swrCtx);
        m_audioStreamIndex = -1;
    }


    // ------------------- 生产者-消费者核心逻辑 -------------------

    // [生产者] 解码线程
    void MusicPlayer::Impl::decoderLoop() {
        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();

        while (!m_stopRequested) {
            // 处理暂停
            {
                std::unique_lock<std::mutex> lock(m_controlMutex);
                m_controlCondVar.wait(lock, [this] { return m_state != PlayerState::Paused || m_stopRequested; });
            }
            if (m_stopRequested)
                break;

            // 队列背压控制
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCondVar.wait(lock, [this] { return m_frameQueue.size() < MAX_QUEUE_SIZE || m_stopRequested; });
            }
            if (m_stopRequested)
                break;

            if (av_read_frame(m_formatCtx, packet) >= 0) {
                if (packet->stream_index == m_audioStreamIndex) {
                    if (avcodec_send_packet(m_codecCtx, packet) == 0) {
                        while (avcodec_receive_frame(m_codecCtx, frame) == 0) {
                            auto audioFrame = std::make_shared<AudioFrame>();

                            // 重采样
                            uint8_t **dst_data = nullptr;
                            int dst_linesize;
                            int dst_nb_samples =
                                    av_rescale_rnd(swr_get_delay(m_swrCtx, m_codecCtx->sample_rate) + frame->nb_samples,
                                                   m_codecCtx->sample_rate, m_codecCtx->sample_rate, AV_ROUND_UP);
                            av_samples_alloc_array_and_samples(&dst_data, &dst_linesize,
                                                               m_deviceConfig.playback.channels, dst_nb_samples,
                                                               AV_SAMPLE_FMT_FLT, 0);

                            int actual_dst_nb_samples = swr_convert(m_swrCtx, dst_data, dst_nb_samples,
                                                                    (const uint8_t **) frame->data, frame->nb_samples);

                            int data_size = actual_dst_nb_samples * m_deviceConfig.playback.channels *
                                            av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT);
                            audioFrame->data.resize(data_size);
                            memcpy(audioFrame->data.data(), dst_data[0], data_size);

                            av_freep(&dst_data[0]);
                            av_freep(&dst_data);

                            // 推入队列
                            {
                                std::lock_guard<std::mutex> lock(m_queueMutex);
                                m_frameQueue.push(audioFrame);
                            }
                            m_queueCondVar.notify_one();
                        }
                    }
                }
                av_packet_unref(packet);
            } else {
                // 文件读取完毕
                m_stopRequested = true;
                m_logger->info("文件解码完毕");
                break;
            }
        }

        av_packet_free(&packet);
        av_frame_free(&frame);
        m_logger->info("解码线程退出");
    }

    // [消费者] 音频回调处理
    void MusicPlayer::Impl::processPlaybackFrames(void *pOutput, ma_uint32 frameCount) {
        uint8_t *pOutputU8 = static_cast<uint8_t *>(pOutput);
        ma_uint32 framesToWrite = frameCount;
        ma_uint32 totalFramesWritten = 0;
        int bytes_per_frame = ma_get_bytes_per_frame(m_deviceConfig.playback.format, m_deviceConfig.playback.channels);

        while (framesToWrite > 0) {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            // 等待队列中有数据，设置一个短暂的超时以防解码器卡住时音频线程死锁
            if (m_queueCondVar.wait_for(lock, std::chrono::milliseconds(10),
                                        [this] { return !m_frameQueue.empty() || m_stopRequested; })) {
                if (m_stopRequested) {
                    break;
                }

                auto &audioFrame = m_frameQueue.front();
                size_t bytes_available = audioFrame->data.size() - audioFrame->consumed_bytes;
                size_t bytes_to_copy = std::min((size_t) framesToWrite * bytes_per_frame, bytes_available);

                memcpy(pOutputU8 + (totalFramesWritten * bytes_per_frame),
                       audioFrame->data.data() + audioFrame->consumed_bytes, bytes_to_copy);

                audioFrame->consumed_bytes += bytes_to_copy;
                ma_uint32 framesCopied = bytes_to_copy / bytes_per_frame;
                framesToWrite -= framesCopied;
                totalFramesWritten += framesCopied;

                if (audioFrame->consumed_bytes >= audioFrame->data.size()) {
                    m_frameQueue.pop();
                }

            } else {
                // 超时，队列为空，填充静音
                break;
            }
        }

        // 如果数据不够，用静音填充剩余部分
        if (totalFramesWritten < frameCount) {
            ma_uint32 framesToSilence = frameCount - totalFramesWritten;
            memset(pOutputU8 + (totalFramesWritten * bytes_per_frame), 0, framesToSilence * bytes_per_frame);
        }
    }

} // namespace MusicEngine
