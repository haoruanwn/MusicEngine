#pragma once

#include <memory>
#include "Music.h"

namespace MusicEngine {

    enum class PlayerState {
        Stopped,
        Playing,
        Paused
    };

    class MusicPlayer {
    public:
        MusicPlayer();
        ~MusicPlayer();

        // 禁止拷贝和赋值
        MusicPlayer(const MusicPlayer&) = delete;
        MusicPlayer& operator=(const MusicPlayer&) = delete;

        // 播放一首新歌
        void play(const Music& music);

        // 停止当前播放
        void stop();

        // 暂停当前播放
        void pause();

        // 从暂停处继续播放
        void resume();

        // 获取当前播放状态
        PlayerState getState() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

} // namespace MusicEngine