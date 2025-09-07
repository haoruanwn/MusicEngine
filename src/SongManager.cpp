#include "SongManager.h"
#include <algorithm>
#include <atomic>
#include <format>
#include <string>
#include <future>
#include <mutex>
#include "SongParser.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


// Impl结构体，用于在对外暴露的头文件中隐藏私有成员
struct SongManager::Impl {
    std::vector<Song> m_songDatabase;
    mutable std::mutex m_dbMutex;
    std::future<void> m_scanFuture;
    std::atomic<bool> m_isScanning{false};
    std::vector<std::filesystem::path> m_directoryPaths;
    std::shared_ptr<spdlog::logger> m_logger;

    // 支持的音乐文件拓展名
    std::vector<std::string> supportedExtensions = {".mp3", ".m4a", ".flac", ".wav", ".ogg"};

    // Impl的构造函数
    Impl() {}
};


SongManager::SongManager() : pimpl(std::make_unique<Impl>()) {
    // 初始状态不是扫描中
    pimpl->m_isScanning = false;

    // 初始化SongManager的日志
    pimpl->m_logger = spdlog::stdout_color_mt("SongManager");
    pimpl->m_logger->set_level(spdlog::level::info);
    pimpl->m_logger->info("SongManager initialized.");

    // 初始化 SongParser 的日志
    SongParser::logger_init();
}

// 实现析构函数
SongManager::~SongManager() {
    // 如果程序退出时，后台扫描仍在进行，则等待其完成
    if (pimpl->m_scanFuture.valid()) {
        pimpl->m_scanFuture.wait();
    }
}

SongManager &SongManager::getInstance() {
    static SongManager instance;
    return instance;
}

bool SongManager::startScan(const std::function<void(size_t)> &onScanFinished) {
    // 检查目录路径是否已设置
    if (pimpl->m_directoryPaths.empty()) {
        pimpl->m_logger->error("错误: 目录路径未设置。");
        return false;
    }

    // 检查是否已有扫描任务正在进行
    if (pimpl->m_isScanning) {
        pimpl->m_logger->error("错误: 上一个扫描任务仍在进行中。");
        return false;
    }

    pimpl->m_isScanning = true; // 设置扫描标志

    // 复制一份路径，确保在异步任务中使用的路径不会被修改
    auto pathsToScan = pimpl->m_directoryPaths;

    // 调用std::async启用一个异步任务
    pimpl->m_scanFuture = std::async(std::launch::async, [this, pathsToScan, onScanFinished]() {
        pimpl->m_logger->info("后台扫描开始...");

        std::vector<Song> newDatabase;


        for (const auto &dirPath: pathsToScan) {
            pimpl->m_logger->info("正在扫描: {}", dirPath.string());
            try {
                // recursive_directory_iterator 必须在循环内部对单个路径使用
                for (const auto &entry: std::filesystem::recursive_directory_iterator(dirPath)) {
                    if (entry.is_regular_file()) {

                        // 打印出目前正在处理的文件名
                        pimpl->m_logger->info("正在处理: {}", entry.path().string());

                        std::string extension = entry.path().extension().string();
                        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                        // 检查文件扩展名是否在支持的列表中
                        for (const auto &supExt: pimpl->supportedExtensions) {
                            if (extension == supExt) {

                                // 解析文件并添加到数据库
                                if (auto songOpt = SongParser::createSongFromFile(entry.path())) {
                                    newDatabase.push_back(*songOpt);
                                }
                                break;
                            }
                        }
                    }
                }
            } catch (const std::filesystem::filesystem_error &e) {
                pimpl->m_logger->error("文件系统错误: {}", e.what());
            } catch (...) {
                pimpl->m_logger->error("扫描目录时发生未知错误: {}", dirPath.string());
            }
        }

        size_t count = newDatabase.size();
        pimpl->m_logger->info("扫描完成, 共找到 {} 首歌曲.", count);

        {
            std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);
            pimpl->m_songDatabase = std::move(newDatabase);
        }

        // 调用回调函数通知扫描完成
        if (onScanFinished) {
            onScanFinished(count);
        }

        pimpl->m_isScanning = false;
    });

    return true;
}


bool SongManager::isScanning() const { return pimpl->m_isScanning; }

std::vector<Song> SongManager::getAllSongs() const {
    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);
    return pimpl->m_songDatabase;
}

std::vector<Song> SongManager::searchSongs(const std::string &query) const {
    // 检查目录路径是否已设置
    if (pimpl->m_directoryPaths.empty()) {
        pimpl->m_logger->error("错误: 目录路径未设置。");
        return {};
    }

    std::vector<Song> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);
    for (const auto &song: pimpl->m_songDatabase) {
        std::string lowerTitle = song.title;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);

        if (lowerTitle.find(lowerQuery) != std::string::npos) {
            results.push_back(song);
        }
    }
    return results;
}

std::vector<std::string> SongManager::getSongNames() const {
    std::vector<std::string> results;

    if (pimpl->m_directoryPaths.empty()) {
        pimpl->m_logger->error("错误: 目录路径未设置。");
        return {};
    }

    for (const auto &song: pimpl->m_songDatabase) {
        results.push_back(song.filePath.filename().string());
    }

    return results;
}

// setDirectoryPath 方法设定中只能执行一次，所以每次执行的时候清理之前的
void SongManager::setDirectoryPath(const std::vector<std::filesystem::path> &directoryPaths) {
    pimpl->m_directoryPaths = directoryPaths;
}

void SongManager::setDirectoryPath(const std::filesystem::path &directoryPath) {
    pimpl->m_directoryPaths.push_back(directoryPath);
}

void SongManager::setDirectoryPath(std::initializer_list<std::filesystem::path> directoryPaths) {
    pimpl->m_directoryPaths = directoryPaths;
}


bool SongManager::exportDatabaseToFile(const std::filesystem::path &outputPath) const {
    pimpl->m_logger->info("请求导出数据库到文件: {}", outputPath.string());

    // 动态创建专用于文件输出的logger
    std::shared_ptr<spdlog::logger> file_logger;
    try {
        // 使用文件名作为logger名，确保唯一性。如果logger已存在，spdlog会抛异常。
        auto file_sink =
                std::make_shared<spdlog::sinks::basic_file_sink_mt>(outputPath.string(), true); // true表示覆盖旧文件
        file_logger = std::make_shared<spdlog::logger>("database_exporter", file_sink);

        // 设置文件日志的格式，不需要包含 logger 名和级别
        file_logger->set_pattern("%v");
        file_logger->set_level(spdlog::level::info);

    } catch (const spdlog::spdlog_ex &ex) {
        pimpl->m_logger->error("创建导出日志文件失败: {}. 错误: {}", outputPath.string(), ex.what());
        return false;
    }

    // 锁定数据库，保证线程安全
    std::lock_guard<std::mutex> lock(pimpl->m_dbMutex);

    if (pimpl->m_songDatabase.empty()) {
        pimpl->m_logger->warn("数据库为空，没有内容可以导出。");
        file_logger->info("--- 数据库为空 ---");
        return true; // 操作本身是成功的
    }

    // 使用std::format把时间信息格式化为字符串
    const auto now = std::chrono::system_clock::now();
    const std::string formatted_time = std::format("{:%Y-%m-%d %H:%M:%S}", now);

    file_logger->info("--- 歌曲数据库导出 ---");
    file_logger->info("共 {} 首歌曲", pimpl->m_songDatabase.size());
    file_logger->info("导出时间: {}", formatted_time);
    file_logger->info("----------------------\n");


    // 遍历数据库并格式化输出
    for (const auto &song: pimpl->m_songDatabase) {
        auto format_field = [](const std::string &value) { return value.empty() ? "未知" : value; };

        // 格式化输出字符串
        std::string song_info =
                fmt::format("标题: {}\n"
                            "艺术家: {}\n"
                            "专辑: {}\n"
                            "流派: {}\n"
                            "年份: {}\n"
                            "时长: {} 秒\n"
                            "文件路径: {}\n"
                            "封面大小: {} 字节\n"
                            "----------------------",
                            format_field(song.title), format_field(song.artist), format_field(song.album),
                            format_field(song.genre), song.year == 0 ? "未知" : std::to_string(song.year),
                            song.duration, song.filePath.string(), song.coverArt.size());

        // 写入文件
        file_logger->info(song_info);
    }

    pimpl->m_logger->info("数据库成功导出到: {}", outputPath.string());
    return true;
}

void SongManager::setSupportedExtensions(const std::vector<std::string> &extensions) {
    if (extensions.empty()) {
        pimpl->m_logger->warn("警告: 尝试设置空的支持扩展名列表，保持现有设置不变。");
        return;
    }

    // 清理并设置新的扩展名列表
    pimpl->supportedExtensions.clear();
    for (const auto &ext: extensions) {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        if (!lowerExt.empty() && lowerExt[0] != '.') {
            lowerExt = "." + lowerExt; // 确保扩展名前有点
        }
        pimpl->supportedExtensions.push_back(lowerExt);
    }

    pimpl->m_logger->info("支持的音乐文件扩展名已更新: ");

    // 打印当前支持的扩展名列表
    for (const auto &ext: pimpl->supportedExtensions) {
        pimpl->m_logger->info(" - {}", ext);
    }
}

