# examples/CMakeLists.txt
project(MusicManagerExamples)

message(STATUS "Building the MusicManager examples")

add_executable(${PROJECT_NAME}
        basic_usage.cpp
)

target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        MusicEngine
        spdlog::spdlog
)