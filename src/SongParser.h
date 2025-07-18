// SongParser.h

#pragma once

// 基础库
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// 我们的 Song 数据结构
#include "song.h"

// --- TagLib 核心和各格式的头文件 ---
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

// MP3 支持
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>

// M4A (MP4) 支持
#include <taglib/mp4coverart.h>
#include <taglib/mp4file.h>
#include <taglib/mp4item.h>
#include <taglib/mp4tag.h>

// FLAC 支持 (新增)
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>


namespace SongParser {

    /**
     * @brief 从一个音频文件中解析元数据和封面.
     * (函数定义在头文件中，需要声明为 inline)
     *
     * @param filePath 指向音频文件的 std::filesystem::path 对象.
     * @return 如果解析成功, 返回一个包含 Song 数据的 std::optional; 否则返回 std::nullopt.
     */
    inline std::optional<Song> createSongFromFile(const std::filesystem::path &filePath) {
        // 使用C风格字符串路径创建FileRef，兼容性更好
        TagLib::FileRef fileRef(filePath.c_str());

        // 检查文件是否有效，以及是否包含标签
        if (fileRef.isNull() || !fileRef.tag()) {
            std::cerr << "Error: Could not read tags from file: " << filePath << std::endl;
            return std::nullopt;
        }

        Song song;
        TagLib::Tag *tag = fileRef.tag();

        // -- 1. 读取通用标签 (这部分代码对所有格式都有效) --
        song.title = tag->title().to8Bit(true);
        song.artist = tag->artist().to8Bit(true);
        song.album = tag->album().to8Bit(true);
        song.genre = tag->genre().to8Bit(true);
        if (tag->year() != 0)
            song.year = tag->year();
        song.filePath = filePath;

        if (fileRef.audioProperties()) {
            // 获取以秒为单位的时长
            song.duration = fileRef.audioProperties()->lengthInSeconds();
        }

        // -- 2. 读取封面 (这部分是格式相关的) --
        std::string extension = filePath.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == ".mp3") {
            TagLib::MPEG::File mpegFile(filePath.c_str(), false);
            if (mpegFile.isValid() && mpegFile.hasID3v2Tag()) {
                if (auto id3v2tag = mpegFile.ID3v2Tag()) {
                    const auto frameList = id3v2tag->frameList("APIC");
                    if (!frameList.isEmpty()) {
                        if (auto picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front())) {
                            song.coverArt.assign(picFrame->picture().data(),
                                                 picFrame->picture().data() + picFrame->picture().size());
                            song.coverArtMimeType = picFrame->mimeType().to8Bit(true);
                        }
                    }
                }
            }
        } else if (extension == ".m4a") {
            TagLib::MP4::File mp4File(filePath.c_str(), false);
            if (mp4File.isValid() && mp4File.hasMP4Tag()) {
                if (auto mp4Tag = mp4File.tag()) {
                    // 在MP4中，封面存储在名为 "covr" 的 item 中
                    if (mp4Tag->itemMap().contains("covr")) {
                        const TagLib::MP4::Item coverItem = mp4Tag->itemMap()["covr"];
                        const TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
                        if (!coverArtList.isEmpty()) {
                            const TagLib::MP4::CoverArt &coverArt = coverArtList.front();
                            song.coverArt.assign(coverArt.data().data(),
                                                 coverArt.data().data() + coverArt.data().size());
                            // MP4 封面格式通常是 JPEG 或 PNG
                            if (coverArt.format() == TagLib::MP4::CoverArt::JPEG) {
                                song.coverArtMimeType = "image/jpeg";
                            } else if (coverArt.format() == TagLib::MP4::CoverArt::PNG) {
                                song.coverArtMimeType = "image/png";
                            }
                        }
                    }
                }
            }
        } else if (extension == ".flac") { // <-- 新增对 FLAC 的支持
            TagLib::FLAC::File flacFile(filePath.c_str(), false);
            if (flacFile.isValid()) {
                // FLAC通过pictureList()获取封面列表
                const auto picList = flacFile.pictureList();
                if (!picList.isEmpty()) {
                    TagLib::FLAC::Picture *flacPic = picList.front();
                    song.coverArt.assign(flacPic->data().data(), flacPic->data().data() + flacPic->data().size());
                    song.coverArtMimeType = flacPic->mimeType().to8Bit(true);
                }
            }
        }

        return song;
    }

} // namespace SongParser
