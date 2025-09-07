cmake_minimum_required(VERSION 3.16)
project(SongManager)

# Select whether to build example programs
option(SONGMANAGER_BUILD_EXAMPLES "Build the example programs" OFF)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

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
    # If the option is ON, add the examples directory as a subdirectory
    add_subdirectory(examples)
    message(STATUS "Building examples...")
endif()


# According to whether this is the main project or a subproject, set up install rules and linking
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
