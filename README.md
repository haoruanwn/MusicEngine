## SongManager: A Library for Music File Discovery, Management, and Parsing

English | [中文](./README_CN.md)

### Introduction

`SongManager` is a modern C++ library focused on music file management and metadata parsing. It offers efficient asynchronous file scanning, powerful metadata and album art parsing, designed to simplify the music processing workflow for developers.

### ✨ Features

| Feature                       | Description                                                  |
| ----------------------------- | ------------------------------------------------------------ |
| **Asynchronous Scanning**     | Scans all music files in a specified directory asynchronously and non-blockingly, suitable for large music libraries. |
| **Powerful Metadata Parsing** | Based on `FFmpeg`, it supports parsing metadata (like title, artist, album, year) from various popular audio formats. |
| **Album Art Extraction**      | Capable of extracting embedded album art image data from audio files. |
| **Modern C++ Design**         | Adopts the `C++20` standard, utilizing modern features like `std::filesystem`, smart pointers, and asynchronous programming to ensure code robustness and maintainability. |
| **Flexible Dependency**       | Supports both `git submodule` for source integration and the `install/find_package` approach for flexible usage. |
| **High-Performance Logging**  | Integrates `spdlog` to provide a customizable and efficient logging system for easy debugging and tracking. |

### 🚀 Build and Installation

#### Prerequisites

`SongManager` dynamically links against the LGPL components of FFmpeg. Please ensure that the FFmpeg development packages are installed in your environment before compiling.

- **Debian / Ubuntu**

  ```bash
  sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev
  ```

- **RedHat / Fedora / CentOS**

  ```bash
  sudo dnf install ffmpeg-devel
  ```

#### Method 1: Git Submodule Integration (Recommended)

With this method, the `spdlog` library is linked upwards as a public dependency, allowing you to call it directly in your own project.

1. **Add the submodule**:

   ```bash
   git submodule add https://github.com/haoruanwn/SongManager.git your_path/SongManager
   git submodule update --init --recursive
   ```

2. Modify your CMakeLists.txt:

   Add the subdirectory and link the library in your main project's CMakeLists.txt.

   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(MyAwesomeProject)
   
   # Require C++20 Standard
   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   
   # Add the SongManager subdirectory
   add_subdirectory(your_path/SongManager)
   
   add_executable(${PROJECT_NAME} main.cpp)
   
   # Link against the SongManager library
   target_link_libraries(${PROJECT_NAME} PRIVATE SongManager)
   ```

#### Method 2: Standalone Build and Installation

1. **Clone the repository**:

   ```bash
   git clone https://github.com/haoruanwn/SongManager.git
   cd SongManager
   ```

2. **Configure and Build**:

   ```bash
   # Configure the project and specify the installation path
   cmake --preset Release -DCMAKE_INSTALL_PREFIX=~/local/songmanager_install
   
   # Build the project
   cmake --build --preset Release
   
   # Install the project
   cmake --install build
   ```

3. Use find_package in your project:

   Use find_package in your CMakeLists.txt to find and link SongManager.

   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(MyAwesomeProject)
   
   set(CMAKE_CXX_STANDARD 20)
   
   # Specify the installation path for SongManager
   set(SongManager_DIR ~/local/songmanager_install/lib/cmake/SongManager)
   
   find_package(SongManager REQUIRED)
   
   add_executable(${PROJECT_NAME} main.cpp)
   
   target_link_libraries(${PROJECT_NAME} PRIVATE SongManager::SongManager)
   ```

> **Note**: For cross-compilation, please use the `-DCMAKE_TOOLCHAIN_FILE` argument during the configuration step to specify your toolchain file.

### Credits and Acknowledgements

- **FFmpeg**: [FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg)
- **spdlog**: [gabime/spdlog](https://github.com/gabime/spdlog)