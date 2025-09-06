cmake_minimum_required(VERSION 3.16)
project(SongManager)


# 选择是否构建示例程序
option(SONGMANAGER_BUILD_EXAMPLES "Build the example programs" ON)


# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} STATIC
        src/SongManager.cpp
        src/SongParser.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    PRIVATE
        spdlog::spdlog
        PkgConfig::FFMPEG
)


if(SONGMANAGER_BUILD_EXAMPLES)
    # 如果选项为 ON，则添加 examples 目录作为一个子构建
    add_subdirectory(examples)
    message(STATUS "Building examples...")
endif()

# 安装规则
include(CMakePackageConfigHelpers)
# 根据模板 SongManagerConfig.cmake.in 生成 SongManagerConfig.cmake
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SongManagerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SongManagerConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SongManager
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

# 安装生成的Config文件
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SongManagerConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SongManager
)