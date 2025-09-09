add_library(MusicEngine STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/music_manager/music_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/music_manager/cover_art_cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/music_manager/music_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/music_player/music_player.cpp

)

target_include_directories(MusicEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> # 将include目录暴露给使用者
        $<INSTALL_INTERFACE:include>
        
    PRIVATE
        music_manager/
        music_player/
        ${CMAKE_SOURCE_DIR}/third_party/miniaudio


)

target_link_libraries(MusicEngine PRIVATE
    PkgConfig::FFMPEG
)