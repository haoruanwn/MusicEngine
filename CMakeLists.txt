cmake_minimum_required(VERSION 3.16)
project(music_system)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        taglib # 为这个依赖内容起一个名字
        GIT_REPOSITORY https://github.com/taglib/taglib.git
        GIT_TAG        v2.1.1 # 锁定到一个稳定版本，保证可重复构建
)

# 3. 在获取并构建它之前，可以先设置它的内部选项
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static libraries")
set(BUILD_EXAMPLES OFF CACHE BOOL "Don't build examples")
set(BUILD_TESTING OFF CACHE BOOL "Don't build tests")

# 4. 这条命令会触发下载（如果需要）并像 add_subdirectory 一样处理它
#    它会自动处理好所有依赖和路径问题
FetchContent_MakeAvailable(taglib)

FetchContent_GetProperties(taglib)

add_executable(${PROJECT_NAME} main.cpp
        src/SongParser.h
        src/SongManager.cpp
        src/SongManager.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        ${taglib_SOURCE_DIR}/taglib
        ${taglib_SOURCE_DIR}/taglib/toolkit
        ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        stdc++fs
        tag
)
