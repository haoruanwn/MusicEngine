#pragma once

#include <filesystem>
#include <optional>
#include "Music.h" // Include the definition of the Music struct

namespace music_parser {

    void logger_init();

    /**
     * @brief Parses metadata and cover art from an audio file.
     * @param file_path The path to the music file to be parsed.
     * @return An std::optional<Music> containing the music information if successful; otherwise, std::nullopt.
     */
    std::optional<MusicEngine::Music> create_music_from_file(const std::filesystem::path &file_path);

    /**
     * @brief Extracts cover art data from a file.
     * @param file_path The path to the music file.
     * @return A vector containing the cover art data, or std::nullopt if it fails or does not exist.
     */
    std::optional<std::vector<char>> extract_cover_art_data(const std::filesystem::path &file_path);



} // namespace music_parser