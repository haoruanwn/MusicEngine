add_library(MusicEngine STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MusicManager/MusicManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MusicManager/CoverArtCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MusicManager/MusicParser.cpp

)

target_include_directories(MusicEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> # 将include目录暴露给使用者
        $<INSTALL_INTERFACE:include>
        
    PRIVATE
        MusicManager/
        MusicPlayer/
        ${CMAKE_SOURCE_DIR}/third_party/miniaudio


)

target_link_libraries(MusicEngine PRIVATE
    PkgConfig::FFMPEG
)