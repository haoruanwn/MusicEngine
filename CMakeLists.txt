cmake_minimum_required(VERSION 3.16)
project(MusicManager)

# Global settings and options
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

# Configure Third-Party Dependencies
set(MINIAUDIO_BUILD_EXAMPLES OFF CACHE BOOL "Disable miniaudio examples" FORCE)
set(MINIAUDIO_BUILD_TESTS OFF CACHE BOOL "Disable miniaudio tests" FORCE)
set(MINIAUDIO_NO_DECODING ON CACHE BOOL "Disable all decoders" FORCE) 

# Include subdirectories
# This will execute src/CMakeLists.txt and define the MusicManager target
add_subdirectory(src)

# Conditional linking logic
if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    # Scene 1: Building as the main project
    # We link spdlog here and mark it as PRIVATE
    # because for the final executable (like examples), it's an implementation detail
    message(STATUS "MusicManager is main project. Linking spdlog as PRIVATE.")
    target_link_libraries(MusicEngine PRIVATE spdlog::spdlog)

    # Build examples only when building as the main project
    add_subdirectory(examples/MusicManager_test)
    add_subdirectory(examples/miniaudio_test)
    message(STATUS "Building examples...")
else()
    # Scene 2: Included as a submodule
    # We mark spdlog as PUBLIC.
    # This means any parent target that links against MusicManager will automatically inherit the spdlog dependency.
    # This is the key to "passing it up"!
    message(STATUS "MusicManager is subproject. Linking spdlog as PUBLIC.")
    target_link_libraries(MusicEngine PUBLIC spdlog::spdlog)
endif()


# Installation rules remain at the top level, as this is a project-level decision
if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    message(STATUS "MusicEngine is being built as the main project. Install rules enabled.")

    include(GNUInstallDirs)
    
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS MusicEngine
        EXPORT MusicEngineTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT MusicEngineTargets
        FILE MusicEngineTargets.cmake
        NAMESPACE MusicEngine::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicEngine
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/MusicEngineConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/MusicEngineConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicEngine
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/MusicEngineConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MusicEngine
    )
else()
    message(STATUS "MusicEngine is being built as a subproject. Install rules disabled.")
endif()