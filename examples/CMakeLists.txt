# examples/CMakeLists.txt
project(SongManagerExamples)

message(STATUS "Building examples for SongManager...")

find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)

# 添加一个可执行文件目标
add_executable(${PROJECT_NAME}
        basic_usage.cpp
        opencv_show.cpp
)

target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
)

# 链接到我们项目在顶层定义的 SongManager 库
target_link_libraries(${PROJECT_NAME} PRIVATE
        SongManager
        ${OpenCV_LIBS}
        spdlog::spdlog
)