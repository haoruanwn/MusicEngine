// SongParser.cpp

#include "SongParser.h" // 首先包含自己的头文件，这是一个好习惯

// 包含所有实现所需的 TagLib 内部头文件
#include <taglib/fileref.h>
#include <taglib/tag.h>

// 为了读取封面，需要更具体的头文件
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

// 为了调试，可以包含 iostream
#include <iostream>


// 实现必须也在同一个命名空间下
namespace SongParser
{
    std::optional<Song> createSongFromFile(const std::filesystem::path& filePath)
    {
        // TagLib::FileRef 是一个通用的文件引用类，能自动识别文件类型
        // 注意：TagLib 的路径参数需要 C 风格字符串，所以使用 .c_str()
        TagLib::FileRef fileRef(filePath.c_str());

        // 检查文件是否被 TagLib 成功打开，并且包含可读的标签
        if (fileRef.isNull() || !fileRef.tag())
        {
            std::cerr << "Error: Could not read tags from file: " << filePath << std::endl;
            return std::nullopt; // 返回空 optional 表示失败
        }

        Song song;
        TagLib::Tag* tag = fileRef.tag();

        // -- 1. 读取基本信息 --
        // TagLib::String 不是 std::string，需要转换。to8Bit(true) 表示转为 UTF-8
        song.title = tag->title().to8Bit(true);
        song.artist = tag->artist().to8Bit(true);
        song.album = tag->album().to8Bit(true);
        song.genre = tag->genre().to8Bit(true);
        if (tag->year() != 0) song.year = tag->year();

        // 记录文件路径
        song.filePath = filePath;

        // -- 2. 读取音频属性 (如时长) --
        if (fileRef.audioProperties())
        {
            song.duration = fileRef.audioProperties()->durationInSeconds();
        }

        // -- 3. 读取封面 (以 MP3 的 ID3v2 标签为例) --
        // 注意：这部分是格式相关的，健壮的播放器需要为不同格式编写不同的处理逻辑
        TagLib::MPEG::File mpegFile(filePath.c_str(), false); // 第二个参数 false 表示不读取音频数据，只读标签，更快
        if (mpegFile.isValid() && mpegFile.hasID3v2Tag())
        {
            TagLib::ID3v2::Tag* id3v2tag = mpegFile.ID3v2Tag(true); // 参数 true 确保创建 tag 如果它不存在
            // "APIC" 是内嵌封面的标准 Frame ID
            const auto frameList = id3v2tag->frameList("APIC");
            if (!frameList.isEmpty())
            {
                // 通常我们只取第一张封面
                auto picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
                if (picFrame)
                {
                    song.coverArt.assign(picFrame->picture().data(),
                                         picFrame->picture().data() + picFrame->picture().size());
                    song.coverArtMimeType = picFrame->mimeType().to8Bit(true);
                }
            }
        }
        // TODO: 在这里可以添加对 FLAC, M4A 等格式封面的读取逻辑

        return song; // 返回填充好数据的 Song 对象
    }
} // namespace SongParser
