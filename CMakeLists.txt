cmake_minimum_required(VERSION 3.16)
project(SongManager)


# 选择是否构建示例程序
option(SONGMANAGER_BUILD_EXAMPLES "Build the example programs" ON)
# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
include(cmake/depends.cmake)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} STATIC
        src/SongParser.h
        src/SongManager.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${taglib_SOURCE_DIR}/taglib
        ${taglib_SOURCE_DIR}/taglib/toolkit
        ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    PUBLIC
        stdc++fs
    PRIVATE
        tag
)


if(SONGMANAGER_BUILD_EXAMPLES)
    # 如果选项为 ON，则添加 examples 目录作为一个子构建
    add_subdirectory(examples)
    message(STATUS "Building examples...")
endif()