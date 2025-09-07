# MusicEngine

### A Modern C++ Music Backend Designed for Embedded Linux Platforms

English | [中文](./README_CN.md)

### Introduction

`MusicEngine` is a high-performance music backend solution specifically designed for embedded Linux platforms. Written in modern C++20, it aims to encapsulate the powerful and complex multimedia capabilities of `FFmpeg` into a clean, efficient, and easy-to-integrate C++ interface.

Whether you are building an embedded device with a screen (like a smart speaker or an in-car entertainment system) or a headless audio application, `MusicEngine` is committed to being your stable and reliable music processing core.

### ✨ Feature Roadmap

`MusicEngine` is under active development. Below is the current feature status and future plans.

#### ✅ Implemented Features

| Feature | Description |
| :--- | :--- |
| **Asynchronous File Scanning** | Asynchronously and non-blockingly scans specified directories to efficiently index a vast number of music files. |
| **Core Metadata Parsing** | **[Based on FFmpeg]** Supports parsing metadata (title, artist, album, year, duration, etc.) from various mainstream audio formats. |
| **Album Art Extraction** | **[Based on FFmpeg]** Capable of extracting embedded album art image data from audio files for UI display. |
| **High-Performance Logging** | Integrates `spdlog` to provide an efficient logging system that can be enabled or disabled at compile time, facilitating debugging on resource-constrained devices. |

#### 🛠️ Planned Features

| Feature | Description |
| :--- | :--- |
| **Core Player Engine** | **[Based on FFmpeg]** Implement complete playback control logic, including play, pause, resume, stop, and precise seeking. |
| **Playlist Management** | Provide functionality to create, edit, save, and load playlists. |

### 🚀 Build and Installation

#### Prerequisites

`MusicEngine` dynamically links to FFmpeg's LGPL components. Please ensure that the FFmpeg development packages are installed in your development or target environment before compiling.

  * **Debian / Ubuntu**

    ```bash
    sudo apt install libavformat-dev libavcodec-dev libavutil-dev
    ```

  * **RedHat / Fedora / CentOS**

    ```bash
    sudo dnf install ffmpeg-devel
    ```

#### Method 1: Integrate as a Git Submodule

With this method, the `spdlog` library will be linked upwards as a public dependency, allowing you to call it directly in your own project.

1.  **Add the submodule**:

    ```bash
    git submodule add https://github.com/haoruanwn/MusicEngine.git your_path/MusicEngine
    git submodule update --init --recursive
    ```

2.  **Modify your CMakeLists.txt**:
    Add the subdirectory and link the library in your main project's `CMakeLists.txt`.

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeEmbeddedApp)
    
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    add_subdirectory(your_path/MusicEngine)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    # Link the MusicEngine library
    target_link_libraries(${PROJECT_NAME} PRIVATE MusicEngine)
    ```

#### Method 2: Standalone Compilation and Installation

1.  **Clone the repository**:

    ```bash
    git clone --recursive https://github.com/haoruanwn/MusicEngine.git
    cd MusicEngine
    ```

2.  **Configure and build**:

    ```bash
    # Configure the project, specifying installation to the ~/local/musicengine_install directory
    cmake --preset Release -DCMAKE_INSTALL_PREFIX=~/local/musicengine_install

    # Build the project
    cmake --build --preset Release

    # Install the project
    cmake --install build
    ```

3.  **Use `find_package` in your project**:

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeEmbeddedApp)
    
    set(CMAKE_CXX_STANDARD 20)
    
    # Specify the installation path of MusicEngine
    set(MusicEngine_DIR ~/local/musicengine_install/lib/cmake/MusicEngine)
    
    find_package(MusicEngine REQUIRED)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE MusicEngine::MusicEngine)
    ```

> **Cross-compilation Tip**: If you are cross-compiling for an embedded platform, please specify your toolchain file during the CMake configuration step using the `-DCMAKE_TOOLCHAIN_FILE` parameter.


### Open Source Contributions

We welcome all developers to contribute to MusicEngine. Before starting any work, please read the [Contribution Guidelines](./docs/CONTRIBUTING.md).

### References and Acknowledgements

The implementation of `MusicEngine` would not be possible without these excellent open-source projects:

  * **FFmpeg**: [https://github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg)
  * **spdlog**: [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)

FFmpeg is a leading multimedia framework. `MusicEngine` dynamically links its components, which are licensed under the **GNU Lesser General Public License (LGPL) version 3**.

