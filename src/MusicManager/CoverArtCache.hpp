#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Music.h"

// 

namespace MusicEngine {

    class CoverArtCache {
    public:
        static CoverArtCache& getInstance();

        CoverArtCache(const CoverArtCache&) = delete;
        CoverArtCache& operator=(const CoverArtCache&) = delete;

        /**
         * @brief 获取一首歌的专辑封面。
         *
         * 这是该类的核心接口。它会首先检查缓存，如果未命中，
         * 则会从原始音乐文件中提取封面，存入缓存，然后返回。
         *
         * @param music 需要获取封面的Music对象。
         * @return 一个指向封面数据的共享指针，如果不存在封面则返回nullptr。
         */
        std::shared_ptr<const std::vector<char>> getCoverArt(const Music& music);

    private:
        CoverArtCache();
        ~CoverArtCache();

        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

}