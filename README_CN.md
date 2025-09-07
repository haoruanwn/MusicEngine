# MusicEngine

### 为嵌入式Linux平台设计的现代C++音乐后端

### 简介

`MusicEngine` 是一个专为嵌入式Linux平台打造的、高性能的音乐后端解决方案。它使用现代C++20编写，旨在将强大而复杂的 `FFmpeg` 多媒体能力，封装成一套简洁、高效、易于集成的C++接口。

无论你是在构建一个带有屏幕的嵌入式设备（如智能音箱、车载娱乐系统），还是一个无界面的音频应用，`MusicEngine` 都致力于成为你稳定可靠的音乐处理核心。

### ✨ 功能路线图 (Feature Roadmap)

`MusicEngine` 正处于积极开发中，以下是当前的功能状态和未来规划：

#### ✅ 已实现功能 (Implemented Features)

| 功能 | 描述 |
| :--- | :--- |
| **异步文件扫描** | 非阻塞地异步扫描指定目录，高效索引海量音乐文件。 |
| **核心元数据解析** | **[基于FFmpeg]** 支持解析多种主流音频格式的元数据（标题、艺术家、专辑、年份、时长等）。 |
| **专辑封面提取** | **[基于FFmpeg]** 能够从音频文件中抽取出内嵌的专辑封面图片数据，用于UI显示。 |
| **高性能日志系统** | 集成 `spdlog`，提供可按需编译开关的高效日志，便于在资源受限的设备上进行调试。 |


#### 🛠️ 计划实现功能 (Planned Features)

| 功能 | 描述 |
| :--- | :--- |
| **核心播放器引擎** | **[基于FFmpeg]** 实现完整的播放控制逻辑，包括播放、暂停、恢复、停止和精确跳转(Seek)。 |
| **播放列表管理** | 提供创建、编辑、保存和加载播放列表的功能。 |

### 🚀 构建与安装

#### 依赖环境

`MusicEngine` 动态链接 FFmpeg 的LGPL组件，请在编译前确保您的开发或目标环境中已安装 FFmpeg 开发包。

  * **Debian / Ubuntu**
    
    ```bash
    sudo apt install libavformat-dev libavcodec-dev libavutil-dev
    ```
  * **RedHat / Fedora / CentOS**
    
    ```bash
    sudo dnf install ffmpeg-devel
    ```

#### 方式一：Git Submodule 集成

此方式下，`spdlog` 库会作为公共依赖向上链接，可以在自己的项目中直接调用。

1.  **添加子模块**:

    ```bash
    git submodule add https://github.com/haoruanwn/MusicEngine.git your_path/MusicEngine
    git submodule update --init --recursive
    ```

2.  **修改 CMakeLists.txt**:
    在主项目的 `CMakeLists.txt` 中添加子目录并链接库。

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeEmbeddedApp)
    
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    add_subdirectory(your_path/MusicEngine)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    # 链接 MusicEngine 库
    target_link_libraries(${PROJECT_NAME} PRIVATE MusicEngine)
    ```

#### 方式二：独立编译与安装

1.  **克隆仓库**:

    ```bash
    git clone --recursive https://github.com/haoruanwn/MusicEngine.git
    cd MusicEngine
    ```

2.  **配置与构建**:

    ```bash
    # 配置项目，并指定安装到 ~/local/musicengine_install 目录
    cmake --preset Release -DCMAKE_INSTALL_PREFIX=~/local/musicengine_install

    # 构建项目
    cmake --build --preset Release

    # 安装项目
    cmake --install build
    ```

3.  **在项目中使用 `find_package`**:

    ```cmake
    cmake_minimum_required(VERSION 3.16)
    project(MyAwesomeEmbeddedApp)
    
    set(CMAKE_CXX_STANDARD 20)
    
    # 指定 MusicEngine 的安装路径
    set(MusicEngine_DIR ~/local/musicengine_install/lib/cmake/MusicEngine)
    
    find_package(MusicEngine REQUIRED)
    
    add_executable(${PROJECT_NAME} main.cpp)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE MusicEngine::MusicEngine)
    ```

> **交叉编译提示**: 如果您正在为嵌入式平台进行交叉编译，请在CMake配置阶段通过 `-DCMAKE_TOOLCHAIN_FILE` 参数引入您的工具链文件。

### 开源贡献
欢迎任何开发者来为 MusicEngine 贡献代码，在进行工作之前，请阅读[贡献指南](./docs/CONTRIBUTING.md)。


### 引用与致谢

`MusicEngine` 的实现离不开以下优秀的开源项目：

  * **FFmpeg**: [https://github.com/FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg) 
  * **spdlog**: [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)

FFmpeg 是一个领先的多媒体框架。`MusicEngine` 动态链接了其组件，这些组件在 **GNU Lesser General Public License (LGPL) version 3**  许可下使用
