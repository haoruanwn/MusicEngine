#include "opencv_show.hpp"

/**
 * @brief 使用OpenCV显示带有歌曲信息的专辑封面
 * @param song 要显示的歌曲对象
 */
void displaySongWithCover(const Song &song) {
    // 检查是否有封面数据
    if (song.coverArt.empty()) {
        std::cout << "歌曲 '" << song.title << "' 没有封面信息。" << std::endl;
        return;
    }

    // 封面数据解码成OpenCV的Mat对象
    std::vector<uchar> img_data(song.coverArt.begin(), song.coverArt.end());
    cv::Mat image = cv::imdecode(img_data, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "错误: 无法解码歌曲 '" << song.title << "' 的封面图像。" << std::endl;
        return;
    }

    // 要绘制的文本信息
    std::string title_text = "Title: " + (song.title.empty() ? "Unknown" : song.title);
    std::string artist_text = "Artist: " + (song.artist.empty() ? "Unknown" : song.artist);
    std::string album_text = "Album: " + (song.album.empty() ? "Unknown" : song.album);

    // 在图像上绘制文本
    // 为了让文字更清晰，我们可以在文字下面先绘制一个半透明的背景矩形
    cv::Mat overlay;
    image.copyTo(overlay);
    cv::rectangle(overlay, cv::Rect(10, 10, 400, 100), cv::Scalar(0, 0, 0), -1);
    double alpha = 0.5; // 背景透明度
    cv::addWeighted(overlay, alpha, image, 1 - alpha, 0, image);

    // 设置字体、颜色等
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.7;
    int thickness = 2;
    cv::Scalar color = cv::Scalar(255, 255, 255); // 白色

    // 使用 putText 函数绘制文字
    cv::putText(image, title_text, cv::Point(20, 40), fontFace, fontScale, color, thickness);
    cv::putText(image, artist_text, cv::Point(20, 70), fontFace, fontScale, color, thickness);
    cv::putText(image, album_text, cv::Point(20, 100), fontFace, fontScale, color, thickness);

    // 5. 创建窗口并显示图像
    std::string window_name = "Song Info: " + song.title;
    cv::imshow(window_name, image);

    // 6. 等待用户按键后关闭窗口
    std::cout << "\n按任意键关闭封面窗口..." << std::endl;
    cv::waitKey(0);
    cv::destroyWindow(window_name);
}