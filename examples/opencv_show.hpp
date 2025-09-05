#pragma once

#include "Song.h"
#include <opencv2/opencv.hpp>


/**
 * @brief 使用OpenCV显示带有歌曲信息的专辑封面
 * @param song 要显示的歌曲对象
 */
void displaySongWithCover(const Song &song);