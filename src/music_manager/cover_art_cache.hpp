#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Music.h"

namespace music_engine {

    class CoverArtCache {
    public:
        static CoverArtCache& get_instance();

        CoverArtCache(const CoverArtCache&) = delete;
        CoverArtCache& operator=(const CoverArtCache&) = delete;

        /**
         * @brief Retrieves the cover art for a music.
         *
         * This is the core interface of the class. It first checks the cache.
         * If there is a cache miss, it extracts the cover from the original music file,
         * stores it in the cache, and then returns it.
         *
         * @param music The Music object for which to retrieve the cover art.
         * @return A shared pointer to the cover art data, or nullptr if no cover art exists.
         */
        std::shared_ptr<const std::vector<char>> get_cover_art(const Music& music);

    private:
        CoverArtCache();
        ~CoverArtCache();

        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

}