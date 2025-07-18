cmake_minimum_required(VERSION 3.16)
project(SongManager)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        taglib
        GIT_REPOSITORY https://github.com/taglib/taglib.git
        GIT_TAG        v2.1.1
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static libraries")
set(BUILD_EXAMPLES OFF CACHE BOOL "Don't build examples")
set(BUILD_TESTING OFF CACHE BOOL "Don't build tests")
FetchContent_MakeAvailable(taglib)
FetchContent_GetProperties(taglib)


#add_executable(${PROJECT_NAME} main.cpp
#        src/internal/SongParser.h
#        src/SongManager.cpp
#        include/SongManager.h
#)

add_library(${PROJECT_NAME} STATIC
        src/internal/SongParser.h
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

# 选择是否构建示例程序
option(SONGMANAGER_BUILD_EXAMPLES "Build the example programs" ON)
if(SONGMANAGER_BUILD_EXAMPLES)
    # 如果选项为 ON，则添加 examples 目录作为一个子构建
    add_subdirectory(examples)
    message(STATUS "Building examples...")
endif()