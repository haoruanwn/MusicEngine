#include "ParserLogger.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

// 将定义放在命名空间内
namespace ParserLog {


    std::shared_ptr<spdlog::logger> logger;

    void init() {
        if (!logger) {
            logger = spdlog::stdout_color_mt("SongParser");
            logger->set_level(spdlog::level::info);
        }
    }

} // namespace ParserLog