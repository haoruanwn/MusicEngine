// SongParser.h

#pragma once

// 基础库
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Song.h"

#include <iostream>
#include "attachedpictureframe.h"
#include "audioproperties.h"
#include "fileref.h"
#include "flacfile.h"
#include "id3v2tag.h"
#include "mp4coverart.h"
#include "mp4file.h"
#include "mp4item.h"
#include "mp4tag.h"
#include "mpegfile.h"
#include "tag.h"


namespace SongParser {

    // 每种音乐类型，提取封面的代码都不一样，但其他部分是通用的
    // 定义一个模板结构体作为策略
    template<typename T>
    struct CoverArtExtractor;
    // 针对不同格式，特化这个结构体

    // mp3
    template<>
    struct CoverArtExtractor<TagLib::MPEG::File> {
        static void extract(const std::filesystem::path &filePath, Song &song) {
            TagLib::MPEG::File file(filePath.c_str(), false);
            if (!file.isValid() || !file.hasID3v2Tag())
                return;

            if (auto id3v2tag = file.ID3v2Tag()) {
                const auto frameList = id3v2tag->frameList("APIC");
                if (!frameList.isEmpty()) {
                    if (auto picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front())) {
                        song.coverArt.assign(picFrame->picture().data(),
                                             picFrame->picture().data() + picFrame->picture().size());
                        if (!picFrame->mimeType().isEmpty())
                            song.coverArtMimeType = picFrame->mimeType().to8Bit(true);
                    }
                }
            }
        }
    };

    // mp4(m4a)
    template<>
    struct CoverArtExtractor<TagLib::MP4::File> {
        static void extract(const std::filesystem::path &filePath, Song &song) {
            TagLib::MP4::File file(filePath.c_str(), false);
            if (!file.isValid() || !file.hasMP4Tag())
                return;

            if (auto mp4Tag = file.tag()) {
                if (mp4Tag->itemMap().contains("covr")) {
                    const auto coverArtList = mp4Tag->itemMap()["covr"].toCoverArtList();
                    if (!coverArtList.isEmpty()) {
                        const auto &coverArt = coverArtList.front();
                        song.coverArt.assign(coverArt.data().data(), coverArt.data().data() + coverArt.data().size());
                        if (coverArt.format() == TagLib::MP4::CoverArt::JPEG) {
                            song.coverArtMimeType = "image/jpeg";
                        } else if (coverArt.format() == TagLib::MP4::CoverArt::PNG) {
                            song.coverArtMimeType = "image/png";
                        }
                    }
                }
            }
        }
    };

    // flac
    template<>
    struct CoverArtExtractor<TagLib::FLAC::File> {
        static void extract(const std::filesystem::path &filePath, Song &song) {
            TagLib::FLAC::File file(filePath.c_str(), false);
            if (!file.isValid())
                return;

            const auto picList = file.pictureList();
            if (!picList.isEmpty()) {
                TagLib::FLAC::Picture *flacPic = picList.front();
                song.coverArt.assign(flacPic->data().data(), flacPic->data().data() + flacPic->data().size());
                if (!flacPic->mimeType().isEmpty())
                    song.coverArtMimeType = flacPic->mimeType().to8Bit(true);
            }
        }
    };

    // 定义一个函数指针类型，用于指向我们的提取策略
    using CoverArtExtractFunc = void (*)(const std::filesystem::path &, Song &);

    // 创建一个从扩展名到提取函数的映射表
    const std::map<std::string, CoverArtExtractFunc> g_coverArtExtractors = {
            {".mp3", &CoverArtExtractor<TagLib::MPEG::File>::extract},
            {".m4a", &CoverArtExtractor<TagLib::MP4::File>::extract},
            {".flac", &CoverArtExtractor<TagLib::FLAC::File>::extract}};


    /**
     * @brief 从一个音频文件中解析元数据和封面.
     * (函数定义在头文件中，需要声明为 inline)
     *
     * @param filePath 指向音频文件的 std::filesystem::path 对象.
     * @return 如果解析成功, 返回一个包含 Song 数据的 std::optional; 否则返回 std::nullopt.
     */
    inline std::optional<Song> createSongFromFile(const std::filesystem::path &filePath) {
        TagLib::FileRef fileRef(filePath.c_str());
        if (fileRef.isNull() || !fileRef.tag()) {
            std::cerr << "Error: Could not read tags from file: " << filePath << std::endl;
            return std::nullopt;
        }

        Song song;
        TagLib::Tag *tag = fileRef.tag();

        // -- 读取通用标签 --
        if (!tag->title().isEmpty())
            song.title = tag->title().to8Bit(true);
        if (!tag->artist().isEmpty())
            song.artist = tag->artist().to8Bit(true);
        if (!tag->album().isEmpty())
            song.album = tag->album().to8Bit(true);
        if (!tag->genre().isEmpty())
            song.genre = tag->genre().to8Bit(true);
        if (tag->year() != 0)
            song.year = tag->year();
        song.filePath = filePath;
        if (fileRef.audioProperties()) {
            song.duration = fileRef.audioProperties()->lengthInSeconds();
        }

        // -- 读取封面 (通过调度器完成) --
        std::string extension = filePath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        // 在映射表中查找对应的提取函数
        auto it = g_coverArtExtractors.find(extension);
        if (it != g_coverArtExtractors.end()) {
            // 如果找到，就调用它
            it->second(filePath, song);
        }

        return song;
    }

} // namespace SongParser
