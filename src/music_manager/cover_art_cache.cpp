#include "cover_art_cache.hpp"
#include <unordered_map>
#include "music_parser.hpp"

namespace MusicEngine {

    struct CoverArtCache::Impl {
        // Memory cache: Key is the file path, Value is a shared pointer to the cover data
        // Using weak_ptr avoids circular references and allows the system to reclaim memory automatically when under pressure
        std::unordered_map<std::string, std::weak_ptr<const std::vector<char>>> memory_cache_;
        std::mutex cache_mutex_;
    };

    // Singleton accessor
    CoverArtCache &CoverArtCache::get_instance() {
        static CoverArtCache instance;
        return instance;
    }

    CoverArtCache::CoverArtCache() : pimpl_(std::make_unique<Impl>()) {}

    CoverArtCache::~CoverArtCache() = default;

    std::shared_ptr<const std::vector<char>> CoverArtCache::get_cover_art(const Music &music) {
        if (!music.has_cover_art) {
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(pimpl_->cache_mutex_);
        const std::string key = music.file_path.string();

        // 1. Check memory cache
        if (auto it = pimpl_->memory_cache_.find(key); it != pimpl_->memory_cache_.end()) {
            if (auto shared_ptr = it->second.lock()) {
                // Cache hit and data is valid
                return shared_ptr;
            }
        }

        // 2. Cache miss, load from file
        if (auto data_opt = music_parser::extract_cover_art_data(music.file_path)) {
            auto shared_ptr = std::make_shared<const std::vector<char>>(std::move(*data_opt));
            // Store in cache
            pimpl_->memory_cache_[key] = shared_ptr;
            return shared_ptr;
        }

        return nullptr; // Extraction failed
    }

} // namespace MusicEngine
