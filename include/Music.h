#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace MusicEngine {

    // Structure to hold music information
    struct Music {
        // Basic metadata
        std::string title; // Music title
        std::string artist; // Artist name
        std::string album; // Album name
        std::string genre; // Genre
        int32_t year = 0; // Release year
        int32_t duration = 0; // Duration in seconds

        // Filesystem path to the music file
        std::filesystem::path filePath;

        // Album cover art (binary data)
        std::vector<char> coverArt;
        std::string coverArtMimeType; // e.g., "image/jpeg"
    };

} // namespace MusicEngine
