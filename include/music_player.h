#pragma once

#include <functional>
#include <memory>
#include <optional>
#include "Music.h"

namespace MusicEngine {

    /**
     * @enum PlayerState
     * @brief Represents the current playback state of the MusicPlayer.
     */
    enum class PlayerState { Stopped, Playing, Paused };

    /**
     * @class MusicPlayer
     * @brief Manages the playback of a single music track.
     *
     * MusicPlayer provides a high-level interface for controlling audio playback,
     * including play, pause, stop, resume, and seek operations. It encapsulates
     * the complexity of audio decoding and device handling in a simple-to-use class.
     * This class uses the Pimpl (Pointer to implementation) idiom.
     */
    class MusicPlayer {
    public:
        /**
         * @brief Constructs a new MusicPlayer instance.
         * The player is initially in the Stopped state.
         */
        MusicPlayer();

        /**
         * @brief Destroys the MusicPlayer instance.
         * Automatically stops playback and releases all associated resources.
         */
        ~MusicPlayer();

        // A MusicPlayer manages unique resources (like threads and audio device handles),
        // so it should not be copyable.
        MusicPlayer(const MusicPlayer &) = delete;
        MusicPlayer &operator=(const MusicPlayer &) = delete;

        /**
         * @brief Starts playback of a new music track.
         * If another track is already playing, it will be stopped before the new one begins.
         * @param music The Music object to be played.
         */
        void play(const MusicEngine::Music &music);

        /**
         * @brief Stops the current playback completely.
         * The player state transitions to Stopped, and the current playback position is lost.
         */
        void stop();

        /**
         * @brief Pauses the current playback.
         * The player state transitions to Paused. Playback can be resumed from the same
         * position using resume(). Has no effect if the player is not in the Playing state.
         */
        void pause();

        /**
         * @brief Resumes playback from the paused state.
         * The player state transitions back to Playing. Has no effect if the player is
         * not in the Paused state.
         */
        void resume();

        /**
         * @brief Gets the current state of the player.
         * @return The current PlayerState (e.g., Stopped, Playing, Paused).
         */
        PlayerState get_state() const;

        /**
         * @brief Gets the total duration of the currently loaded music track.
         * @return The total duration in seconds. Returns 0.0 if no music is loaded.
         */
        double get_duration() const;

        /**
         * @brief Gets the current playback position within the track.
         * @return The current position in seconds from the beginning of the track.
         */
        double get_current_position() const;

        /**
         * @brief Gets the current playback position as an integer percentage.
         * @return The current progress as an integer percentage from 0 to 100.
         */
        int get_current_position_percent() const;

        /**
         * @brief Seeks to a specific time position in the currently playing track.
         * The seek operation is performed asynchronously by the playback thread.
         * @param position_secs The target time in seconds from the beginning of the track.
         * @return std::optional<double> containing the actual requested position if the seek
         * request is accepted, or std::nullopt if the request is ignored (e.g., player is stopped).
         */
        std::optional<double> seek(double position_secs);

        /**
         * @brief Seeks to a specific position using a percentage.
         * Values outside the 0-100 range will be automatically clamped.
         * The seek operation is performed asynchronously.
         * @param percentage The target position as an integer percentage (0-100).
         * @return std::optional<int> containing the actual (clamped) percentage if the seek
         * request is accepted, or std::nullopt if ignored.
         */
        std::optional<int> seek_percent(int percentage);

        /**
         * @brief Sets a callback function to be invoked when playback of a track finishes naturally.
         * @param callback The function to call. It will be invoked from a background thread,
         * so any operations within the callback should be thread-safe.
         */
        void set_on_playback_finished_callback(const std::function<void()>& callback);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace MusicEngine