#pragma once

#include "spdlog/logger.h"
#include <memory>

namespace ParserLog {

    extern std::shared_ptr<spdlog::logger> logger;

    void init();

} // namespace ParserLog