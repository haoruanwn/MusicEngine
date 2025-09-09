

# MusicEngine

### A Modern C++ Music Backend Designed for Embedded Linux Platforms

English | [中文](./README_CN.md)

`MusicEngine` is a modern C++ music backend specifically designed for embedded Linux platforms. Based on the C++20 standard, it encapsulates the multimedia processing capabilities of `FFmpeg` into a concise and efficient C++ interface, aiming to provide a stable and reliable music processing core for embedded devices, with or without a screen.

### ✨ Core Features

`MusicEngine` provides a complete music backend solution through two core singleton classes: `MusicManager` and `MusicPlayer`.

#### 🎶 Music Library Management (`MusicManager`)

`MusicManager` is responsible for the efficient and intelligent management of local music files.

| Feature | Detailed Description |
|---|---|
| **Non-blocking Music Scanning** | - **Asynchronous Processing**: File scanning is performed in a separate background thread, without blocking the main thread. - **Status Query**: The scanning status can be checked at any time using `is_scanning()`. - **Completion Callback**: Supports registering an `on_scan_finished` callback to automatically notify the upper layer upon completion of the scan. |
| **Comprehensive Metadata Parsing** | Utilizes `FFmpeg` to parse various audio formats, extracting core metadata such as **title, artist, album, year, genre, and duration**. |
| **Intelligent Album Art Management** | - **Lazy Loading**: The initial scan only checks for the existence of album art to speed up the scanning process. - **On-demand Extraction & Caching**: Album art data is extracted and automatically cached only upon the first request. - **Automatic Memory Reclamation**: Uses `std::weak_ptr` to manage the cache, automatically releasing memory when the album art is no longer in use. |
| **Flexible Querying & Configuration** | - **Fuzzy Search**: Provides a `search_musics` interface that supports case-insensitive title matching. - **Custom File Types**: Allows setting the file extensions to be scanned via `set_supported_extensions`. - **Data Export**: Supports exporting the music library metadata to a file using `export_database_to_file`. |

#### 🎧 High-Performance Audio Player (`MusicPlayer`)

`MusicPlayer` focuses on providing stable, smooth, and precisely controllable audio playback.

| Feature | Detailed Description |
|---|---|
| **Basic Playback Control** | Provides a complete set of `play`, `pause`, `resume`, and `stop` interfaces. |
| **Precise Playback Control & Status Retrieval** | - **Seek**: Supports seeking by a **specific number of seconds** or by **playback progress percentage**. - **Real-time Progress Reporting**: Can retrieve the current playback progress (in seconds and percentage) in real-time. |
| **Robust Multi-threaded Architecture** | Adopts the classic **producer-consumer model**, decoding audio in a separate background thread and feeding data to the audio device through a buffer queue. |
| **Event Notification Mechanism** | Supports setting a callback via `set_on_playback_finished_callback` to actively notify the application layer when a song finishes playing naturally. |

### ⚙️ System-level Features

| Feature | Description |
|---|---|
| **High-Performance Logging System** | Deeply integrated with `spdlog` for high-performance asynchronous logging. Supports **completely removing** log code in Release builds via a compile switch for zero performance overhead. |
| **Modern C++20 Design** | The entire codebase is written using the C++20 standard, extensively using features like smart pointers, the Pimpl idiom, and RAII to ensure memory safety and high maintainability. |
| **Born for Embedded Platforms** | Focuses on balancing performance and resource consumption. Relies on cross-platform standard libraries like `FFmpeg` for good portability. |

### 🚀 Build and Installation

#### Dependencies

`MusicEngine` dynamically links to FFmpeg's LGPL components. Please ensure that the FFmpeg development packages are installed in your development or target environment before compiling.

  * **Debian / Ubuntu**

    ```bash
    sudo apt install libavformat-dev libavcodec-dev libavutil-dev
    ```

  * **RedHat / Fedora / CentOS**

    ```bash
    sudo dnf install ffmpeg-devel
    ```

#### Method 1: Integrate via Git Submodule

With this method, the `spdlog` library is linked upwards as a public dependency and can be called directly in your own project.

1.  **Add the submodule**:

    ```bash
    git submodule add https://github.com/haoruanwn/MusicEngine.git your_path/MusicEngine
    git submodule update --init --recursive
    ```

2.  **Modify your CMakeLists.txt**:
    In your main project's `CMakeLists.txt`, add the subdirectory and link the library.

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeEmbeddedApp)
    
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    add_subdirectory(your_path/MusicEngine)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    # Link against the MusicEngine library
    target_link_libraries(${PROJECT_NAME} PRIVATE MusicEngine)
    ```

#### Method 2: Compile and Install Independently

1.  **Clone the repository**:

    ```bash
    git clone --recursive https://github.com/haoruanwn/MusicEngine.git
    cd MusicEngine
    ```

2.  **Configure and Build**:

    ```bash
    # Configure the project and specify the installation directory to ~/local/musicengine_install
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
    
    # Specify the installation path for MusicEngine
    set(MusicEngine_DIR ~/local/musicengine_install/lib/cmake/MusicEngine)
    
    find_package(MusicEngine REQUIRED)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE MusicEngine::MusicEngine)
    ```

> **Cross-compilation Tip**: If you are cross-compiling for an embedded platform, please use the `-DCMAKE_TOOLCHAIN_FILE` parameter during the CMake configuration step to include your toolchain file.

### Contributing

All developers are welcome to contribute to MusicEngine. Before you start, please read the [contribution guide](https://www.google.com/search?q=./docs/CONTRIBUTING.md).

### Credits and Acknowledgements

The implementation of `MusicEngine` would not be possible without the following excellent open-source projects:

  * **FFmpeg**: [https://github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg)
  * **spdlog**: [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)
  * **miniaudio**: [https://github.com/mackron/miniaudio](https://github.com/mackron/miniaudio)

FFmpeg is a leading multimedia framework. `MusicEngine` dynamically links its components, which are used under the **GNU Lesser General Public License (LGPL) version 3**.