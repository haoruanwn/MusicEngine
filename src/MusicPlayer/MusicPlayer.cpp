#include "MusicPlayer.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include "Music.h"
#include "MusicManager.h"


extern "C" {
// FFmpeg headers
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

// miniaudio headers
#include "miniaudio.h"
}

namespace MusicEngine {
    // 为了简洁，我们定义一个结构体来存放解码后的音频帧
    struct AudioFrame {
        std::vector<uint8_t> data; // PCM 数据
        // 可以添加其他信息，如时间戳 (PTS)
    };

    struct MusicPlayer::Impl {
        // --- 线程与同步 ---
        std::thread m_decoderThread; // 生产者：解码线程
        std::atomic<bool> m_stopRequested{false}; // 优雅地停止线程

        // --- 状态管理 ---
        std::atomic<PlayerState> m_state{PlayerState::Stopped};
        std::mutex m_controlMutex; // 用于暂停/继续的锁
        std::condition_variable m_controlCondVar; // 用于暂停/继续的条件变量

        // --- 数据缓冲队列 (核心) ---
        std::queue<std::shared_ptr<AudioFrame>> m_frameQueue;
        std::mutex m_queueMutex; // 保护队列的互斥锁
        std::condition_variable m_queueCondVar; // 用于通知队列状态变化的条件变量
        const size_t MAX_QUEUE_SIZE = 100; // 队列最大帧数，防止内存无限增长

        // --- FFmpeg 相关上下文 ---
        // 使用智能指针自动管理资源
        std::unique_ptr<AVFormatContext, decltype(&avformat_close_input)> m_formatCtx{nullptr, &avformat_close_input};
        std::unique_ptr<AVCodecContext, decltype(&avcodec_free_context)> m_codecCtx{nullptr, &avcodec_free_context};
        // 其他如 SwrContext (重采样) 等

        int m_audioStreamIndex = -1;

        // --- 音频输出 ---
        // 这里以 miniaudio 为例，你需要包含它的头文件
        ma_device m_audioDevice;
        ma_device_config m_deviceConfig;

        // --- 当前播放信息 ---
        std::optional<Music> m_currentMusic;
    };


} // namespace MusicEngine
