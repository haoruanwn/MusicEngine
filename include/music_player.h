#pragma once

#include <memory>
#include <optional>
#include "Music.h"

namespace MusicEngine {

    enum class PlayerState { Stopped, Playing, Paused };

    class MusicPlayer {
    public:
        MusicPlayer();
        ~MusicPlayer();

        // Disable copy and assignment
        MusicPlayer(const MusicPlayer &) = delete;
        MusicPlayer &operator=(const MusicPlayer &) = delete;

        // Play a new music
        void play(const MusicEngine::Music &music);

        // Stop the current playback
        void stop();

        // Pause the current playback
        void pause();

        // Resume playback from the paused position
        void resume();

        // Get the current playback state
        PlayerState get_state() const;

        // Get the duration of the current music in seconds
        double get_duration() const;

        // Get the current playback position in seconds
        double get_current_position() const;

        // Get the current playback position as a percentage (0-100)
        int get_current_position_percent() const;

        // Seek to a specific position in the music
        std::optional<double> seek(double position_secs);

        // Seek to a specific position using a percentage (0-100)
        std::optional<int> seek_percent(int percentage);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace MusicEngine
