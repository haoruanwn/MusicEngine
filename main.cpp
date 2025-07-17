// main.cpp
#include <iostream>
#include <string_view>
#include "song.h"
#include "SongParser.h"

void printSongInfo(const Song& song);



int main() {
    const std::filesystem::path music_file {"./music_test"};

    if (!std::filesystem::exists(music_file)) {
        std::cerr << "错误: 文件不存在!" << std::endl;
        return 1;
    }

    // 调用解析函数
    if (auto song_opt = SongParser::createSongFromFile(music_file))
    {
        std::cout << "成功读取歌曲信息" << std::endl;

    }


    return 0;
}

// 一个简单的函数，用来打印歌曲信息
void printSongInfo(const Song& song) {
    std::cout << "===== 歌曲信息 =====" << std::endl;
    std::cout << "标题: " << (song.title.empty() ? "未知" : song.title) << std::endl;
    std::cout << "艺术家: " << (song.artist.empty() ? "未知" : song.artist) << std::endl;
    std::cout << "专辑: " << (song.album.empty() ? "未知" : song.album) << std::endl;
    std::cout << "文件路径: " << song.filePath << std::endl;
    std::cout << "封面大小: " << song.coverArt.size() << " 字节" << std::endl;
    std::cout << "====================" << std::endl;
}