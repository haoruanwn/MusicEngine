## SongManager: 音乐文件检索、管理与解析库

### 简介

`SongManager` 是一个专注于音乐文件管理和元数据解析的现代C++库。它提供了高效的异步文件扫描、强大的元数据与专辑封面解析功能，旨在简化开发者的音乐处理流程。

### ✨ 功能特性

| 功能 | 描述 |
| :--- | :--- |
| **异步文件扫描** | 非阻塞地异步扫描指定目录下的所有音乐文件，适用于大型音乐库。 |
| **强大的元数据解析** | 基于 `FFmpeg`，支持解析多种主流音频格式的元数据（如标题、艺术家、专辑、年份等）。 |
| **专辑封面提取** | 能够从音频文件中抽取出内嵌的专辑封面图片数据。 |
| **现代C++设计** | 采用 `C++20` 标准，利用 `std::filesystem`、智能指针和异步编程等现代化特性，确保代码的健壮性与可维护性。 |
| **灵活的依赖管理** | 支持 `git submodule` 源码集成和 `install/find_package` 两种主流使用方式。 |
| **高性能日志** | 集成 `spdlog`，提供可定制且高效的日志系统，便于调试与追踪。 |

### 🚀 构建与安装

#### 依赖环境

`SongManager` 动态链接 FFmpeg 的LGPL组件，请在编译前确保开发环境中已安装 FFmpeg 开发包。

  * **Debian / Ubuntu**
    ```bash
    sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev
    ```
  * **RedHat / Fedora / CentOS**
    ```bash
    sudo dnf install ffmpeg-devel
    ```

#### 方式一：Git Submodule 集成 

此方式下，`spdlog` 库会作为公共依赖向上链接，您可以在自己的项目中直接调用。

1.  **添加子模块**:

    ```bash
    git submodule add https://github.com/haoruanwn/SongManager.git your_path/SongManager
    git submodule update --init --recursive
    ```

2.  **修改 CMakeLists.txt**:
    在主项目的 `CMakeLists.txt` 中添加子目录并链接库。

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeProject)
    
    # 要求 C++20 标准
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # 添加 SongManager 子目录
    add_subdirectory(your_path/SongManager)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    # 链接 SongManager 库
    target_link_libraries(${PROJECT_NAME} PRIVATE SongManager)
    ```

#### 方式二：独立编译与安装

1.  **克隆仓库**:

    ```bash
    git clone https://github.com/haoruanwn/SongManager.git
    cd SongManager
    ```

2.  **配置与构建**:

    ```bash
    # 配置项目，并指定安装到 ~/local/songmanager_install 目录
    cmake --preset Release -DCMAKE_INSTALL_PREFIX=~/local/songmanager_install

    # 构建项目
    cmake --build --preset Release

    # 安装项目
    cmake --install build
    ```

3.  **在项目中使用 `find_package`**:
    在您的 `CMakeLists.txt` 中使用 `find_package` 来查找并链接 `SongManager`。

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeProject)
    
    set(CMAKE_CXX_STANDARD 20)
    
    # 指定 SongManager 的安装路径
    set(SongManager_DIR ~/local/songmanager_install/lib/cmake/SongManager)
    
    find_package(SongManager REQUIRED)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE SongManager::SongManager)
    ```

> **注意**: 如果进行交叉编译，请在配置阶段通过 `-DCMAKE_TOOLCHAIN_FILE` 参数引入您的工具链文件。

### 引用与致谢

  * **FFmpeg**: [https://github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg)
  * **spdlog**: [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)

