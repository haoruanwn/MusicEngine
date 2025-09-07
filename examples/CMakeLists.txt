# examples/CMakeLists.txt
project(MusicManagerExamples)

message(STATUS "Building examples for MusicManager...")

add_executable(${PROJECT_NAME}
        basic_usage.cpp
)

target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        MusicManager
        spdlog::spdlog
)