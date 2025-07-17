cmake_minimum_required(VERSION 3.31)
project(music_system)

set(CMAKE_CXX_STANDARD 20)


find_package(ZLIB REQUIRED)
find_package(TagLib REQUIRED)

add_executable(${PROJECT_NAME} main.cpp
        lib/SongParser.cpp
        lib/SongParser.h
        lib/SongManager.cpp
        lib/SongManager.h)

target_include_directories(${PROJECT_NAME} PRIVATE
        lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        stdc++fs
        TagLib::TagLib
)