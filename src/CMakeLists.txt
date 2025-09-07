add_library(MusicManager STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/MusicManager/MusicManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MusicManager/CoverArtCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MusicManager/MusicParser.cpp
)

target_include_directories(MusicManager
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> # 将include目录暴露给使用者
        $<INSTALL_INTERFACE:include>
    PRIVATE
        MusicManager/
        MusicPlayer/

)

target_link_libraries(MusicManager PRIVATE
    PkgConfig::FFMPEG
)