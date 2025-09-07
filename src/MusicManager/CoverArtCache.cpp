#include "CoverArtCache.hpp"
#include <unordered_map>
#include "MusicParser.hpp"

namespace MusicEngine {

    struct CoverArtCache::Impl {
        // 内存缓存：Key是文件路径，Value是指向封面数据的共享指针
        // 使用 weak_ptr 可以避免循环引用，并允许系统在内存紧张时自动回收
        std::unordered_map<std::string, std::weak_ptr<const std::vector<char>>> memoryCache;
        std::mutex cacheMutex;
    };

    // Singleton accessor
    CoverArtCache &CoverArtCache::getInstance() {
        static CoverArtCache instance;
        return instance;
    }

    CoverArtCache::CoverArtCache() : pimpl(std::make_unique<Impl>()) {}

    CoverArtCache::~CoverArtCache() = default;

    std::shared_ptr<const std::vector<char>> CoverArtCache::getCoverArt(const Music &music) {
        if (!music.hasCoverArt) {
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(pimpl->cacheMutex);
        const std::string key = music.filePath.string();

        // 1. 检查内存缓存
        if (auto it = pimpl->memoryCache.find(key); it != pimpl->memoryCache.end()) {
            if (auto shared_ptr = it->second.lock()) {
                // 缓存命中且数据有效
                return shared_ptr;
            }
        }

        // 2. 缓存未命中，从文件加载
        if (auto dataOpt = MusicParser::extractCoverArtData(music.filePath)) {
            auto shared_ptr = std::make_shared<const std::vector<char>>(std::move(*dataOpt));
            // 存入缓存
            pimpl->memoryCache[key] = shared_ptr;
            return shared_ptr;
        }

        return nullptr; // 提取失败
    }

} // namespace MusicEngine
