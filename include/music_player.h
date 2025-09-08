#pragma once

#include <memory>
#include "Music.h"

namespace music_engine {

    enum class PlayerState {
        Stopped,
        Playing,
        Paused
    };

    class MusicPlayer {
    public:
        MusicPlayer();
        ~MusicPlayer();

        // Disable copy and assignment
        MusicPlayer(const MusicPlayer&) = delete;
        MusicPlayer& operator=(const MusicPlayer&) = delete;

        // Play a new music
        void play(const music_engine::Music& music);

        // Stop the current playback
        void stop();

        // Pause the current playback
        void pause();

        // Resume playback from the paused position
        void resume();

        // Get the current playback state
        PlayerState get_state() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace music_engine