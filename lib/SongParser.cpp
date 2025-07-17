// SongParser.cpp

#include "SongParser.h"

// 包含所有实现所需的 TagLib 内部头文件
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <iostream>

// -- 包含 MP3 (MPEG) 和 M4A (MP4) 特定的头文件 --
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/mp4item.h>

namespace SongParser {

std::optional<Song> createSongFromFile(const std::filesystem::path& filePath) {
    TagLib::FileRef fileRef(filePath.c_str());

    if (fileRef.isNull() || !fileRef.tag()) {
        std::cerr << "Error: Could not read tags from file: " << filePath << std::endl;
        return std::nullopt;
    }

    Song song;
    TagLib::Tag* tag = fileRef.tag();

    // -- 1. 读取通用标签 (这部分代码对所有格式都有效) --
    song.title = tag->title().to8Bit(true);
    song.artist = tag->artist().to8Bit(true);
    song.album = tag->album().to8Bit(true);
    song.genre = tag->genre().to8Bit(true);
    if(tag->year() != 0) song.year = tag->year();
    song.filePath = filePath;

    if (fileRef.audioProperties()) {
        song.duration = fileRef.audioProperties()->lengthInSeconds();
    }

    // -- 2. 读取封面 (这部分是格式相关的) --
    std::string extension = filePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".mp3") {
        TagLib::MPEG::File mpegFile(filePath.c_str(), false);
        if (mpegFile.isValid() && mpegFile.hasID3v2Tag()) {
            // ... (和之前一样的 MP3 封面读取逻辑)
            if (auto id3v2tag = mpegFile.ID3v2Tag()) {
                const auto frameList = id3v2tag->frameList("APIC");
                if (!frameList.isEmpty()) {
                    if (auto picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front())) {
                        song.coverArt.assign(picFrame->picture().data(), picFrame->picture().data() + picFrame->picture().size());
                        song.coverArtMimeType = picFrame->mimeType().to8Bit(true);
                    }
                }
            }
        }
    }
    else if (extension == ".m4a") {
        TagLib::MP4::File mp4File(filePath.c_str(), false);
        if (mp4File.isValid() && mp4File.hasMP4Tag()) {
            if (auto mp4Tag = mp4File.tag()) {
                // 在MP4中，封面存储在名为 "covr" 的 item 中
                if (mp4Tag->itemMap().contains("covr")) {
                    const TagLib::MP4::Item coverItem = mp4Tag->itemMap()["covr"];
                    const TagLib::MP4::CoverArtList coverArtList = coverItem.toCoverArtList();
                    if (!coverArtList.isEmpty()) {
                        const TagLib::MP4::CoverArt& coverArt = coverArtList.front();
                        song.coverArt.assign(coverArt.data().data(), coverArt.data().data() + coverArt.data().size());
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
    }
    // TODO: 在这里可以添加 else if (extension == ".flac") ... 来支持更多格式

    return song;
}

} // namespace SongParser