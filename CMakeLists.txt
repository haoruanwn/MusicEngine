cmake_minimum_required(VERSION 3.16)
project(SongManager)

# 选择是否构建示例程序
option(SONGMANAGER_BUILD_EXAMPLES "Build the example programs" OFF)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_subdirectory(third_party/spdlog)
find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} STATIC
        src/SongManager.cpp
        src/SongParser.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        PkgConfig::FFMPEG
)

if(SONGMANAGER_BUILD_EXAMPLES)
    # 如果选项为 ON，则添加 examples 目录作为一个子构建
    add_subdirectory(examples)
    message(STATUS "Building examples...")
endif()


# 根据这个项目是主项目还是子项目，设置不同的安装规则和spdlog库的链接方式
if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    message(STATUS "SongManager is being built as the main project. Install rules enabled.")


    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
    )

    target_link_directories(${PROJECT_NAME} PRIVATE
        spdlog::spdlog
    )

    include(GNUInstallDirs)
    
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS ${PROJECT_NAME}
        EXPORT SongManagerTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT SongManagerTargets
        FILE SongManagerTargets.cmake
        NAMESPACE SongManager::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SongManager
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/SongManagerConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/SongManagerConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SongManager
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/SongManagerConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SongManager
    )
else()
    message(STATUS "SongManager is being built as a subproject. Install rules disabled.")

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include
    )

    target_link_directories(${PROJECT_NAME} PUBLIC
        spdlog::spdlog
    )

endif()