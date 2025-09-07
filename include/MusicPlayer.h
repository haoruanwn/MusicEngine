#pragma once

#include "Music.h"

namespace MusicEngine {

    enum class PlaybackState { Stopped, Playing, Paused };

    class MusicPlayer {
    public:
        MusicPlayer();
        ~MusicPlayer();

        // 播放一首新歌
        void play(const Music &music);

        // 暂停当前播放
        void pause();

        // 从暂停处继续播放
        void resume();

        // 跳转到指定时间点 (单位：秒)
        void seek(double seconds);

        // 获取当前播放状态 (例如: Playing, Paused, Stopped)
        PlaybackState getState() const;

        // 获取当前播放进度 (单位：秒)
        double getCurrentPosition() const;

        // 获取总时长 (单位：秒)
        double getDuration() const;

    private:
        // ... 内部实现，例如指向FFmpeg上下文的指针、音频输出流等
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };
} // namespace MusicEngine
