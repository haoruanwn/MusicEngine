cmake_minimum_required(VERSION 3.16)
project(MusicManager)

# Global settings and options
option(MUSICMANAGER_BUILD_EXAMPLES "Build the example programs" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 20)

# Find dependencies
set(SPDLOG_INSTALL ON)
add_subdirectory(third_party/spdlog)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
)

# Include subdirectories
# This will execute src/CMakeLists.txt and define the MusicManager target
add_subdirectory(src)

# Conditional linking logic
if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    # Scene 1: Building as the main project
    # We link spdlog here and mark it as PRIVATE
    # because for the final executable (like examples), it's an implementation detail
    message(STATUS "MusicManager is main project. Linking spdlog as PRIVATE.")
    target_link_libraries(MusicManager PRIVATE spdlog::spdlog)
else()
    # Scene 2: Included as a submodule
    # We mark spdlog as PUBLIC.
    # This means any parent target that links against MusicManager will automatically inherit the spdlog dependency.
    # This is the key to "passing it up"!
    message(STATUS "MusicManager is subproject. Linking spdlog as PUBLIC.")
    target_link_libraries(MusicManager PUBLIC spdlog::spdlog)
endif()

if(MUSICMANAGER_BUILD_EXAMPLES)
    add_subdirectory(examples)
    message(STATUS "Building examples...")
endif()

# Installation rules remain at the top level, as this is a project-level decision
if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    message(STATUS "MusicManager is being built as the main project. Install rules enabled.")

    include(GNUInstallDirs)
    
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS MusicManager
        EXPORT MusicManagerTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT MusicManagerTargets
        FILE MusicManagerTargets.cmake
        NAMESPACE MusicManager::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicManager
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/MusicManagerConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/MusicManagerConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicManager
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/MusicManagerConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicManager
    )
else()
    message(STATUS "MusicManager is being built as a subproject. Install rules disabled.")
endif()