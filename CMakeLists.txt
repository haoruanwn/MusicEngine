cmake_minimum_required(VERSION 3.16)
project(music_system)

set(CMAKE_CXX_STANDARD 20)


find_package(ZLIB REQUIRED)

# --- TagLib 子模块构建 ---
# 2. 在构建 TagLib 之前，我们可以设置一些它的内部选项，来定制化它的构建。
#    这可以减少编译时间并避免构建我们不需要的东西。
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all libraries as static") # 推荐静态链接
set(ENABLE_STATIC_RUNTIME OFF CACHE BOOL "Do not use static runtime") # 通常在Linux上设为OFF
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples")
set(BUILD_TESTING OFF CACHE BOOL "Do not build tests")

# 3. 使用 add_subdirectory 将 TagLib 作为我们项目的一部分来构建。
#    CMake 会进入这个目录，执行它的 CMakeLists.txt 文件。
add_subdirectory(third_party/taglib)


add_executable(${PROJECT_NAME} main.cpp
        src/SongParser.h
        src/SongManager.cpp
        src/SongManager.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
        src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        stdc++fs
        tag
)