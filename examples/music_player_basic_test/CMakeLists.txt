# examples/CMakeLists.txt
project(music_player_basic_test)

message(STATUS "Building the MusicPlayer examples")

add_executable(${PROJECT_NAME}
        main.cpp
)

target_link_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        MusicEngine
        spdlog::spdlog
)