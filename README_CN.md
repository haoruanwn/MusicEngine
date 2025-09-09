# MusicEngine

### 为嵌入式Linux平台设计的现代C++音乐后端
`MusicEngine` 是一个专为嵌入式Linux平台设计的现代C++音乐后端。它基于C++20标准，将`FFmpeg`的多媒体处理能力封装成一套简洁、高效的C++接口，旨在为带屏或无屏的嵌入式设备提供稳定可靠的音乐处理核心。

### ✨ 核心功能

`MusicEngine` 通过 `MusicManager` 和 `MusicPlayer` 两个核心单例类，提供了一套完整的音乐后端解决方案。

#### 🎶 音乐库管理 (`MusicManager`)

`MusicManager` 负责对本地音乐文件进行高效、智能的管理。

| 功能                 | 详细说明                                                     |
| -------------------- | ------------------------------------------------------------ |
| **非阻塞式音乐扫描** | - **异步处理**: 文件扫描在独立后台线程进行，不阻塞主线程。<br>- **状态查询**: 通过 `is_scanning()` 可随时查询扫描状态。<br>- **完成回调**: 支持注册 `on_scan_finished` 回调，在扫描完成时自动通知上层。 |
| **全面的元数据解析** | 利用 `FFmpeg` 解析多种音频格式，提取**标题、艺术家、专辑、年代、流派、时长**等核心元数据。 |
| **智能专辑封面管理** | - **延迟加载**: 初始扫描仅检查封面是否存在，加快扫描速度。<br>- **按需提取与缓存**: 首次请求时才提取封面数据并自动缓存。<br>- **自动内存回收**: 使用 `std::weak_ptr` 管理缓存，当封面不再被使用时自动释放内存。 |
| **灵活的查询与配置** | - **模糊搜索**: 提供 `search_musics` 接口，支持不区分大小写的标题匹配。<br>- **自定义文件类型**: 允许通过 `set_supported_extensions` 设定扫描的文件扩展名。<br>- **数据导出**: 支持通过 `export_database_to_file` 将音乐库元数据导出到文件。 |

#### 🎧 高性能音频播放器 (`MusicPlayer`)

`MusicPlayer` 专注于提供稳定、流畅、可精准控制的音频播放能力。

| 功能                         | 详细说明                                                     |
| ---------------------------- | ------------------------------------------------------------ |
| **基础播放控制**             | 提供完备的 `play`、`pause`、`resume`、`stop` 接口。          |
| **精准的播放控制与状态获取** | - **跳转 (Seek)**: 支持按**指定秒数**或**播放进度百分比**进行跳转。<br>- **实时进度回报**: 能够实时获取当前的播放进度（秒和百分比）。 |
| **健壮的多线程架构**         | 采用经典的**生产者-消费者模型**，在独立的后台线程解码音频，通过缓冲队列为音频设备提供数据。 |
| **事件通知机制**             | 支持通过 `set_on_playback_finished_callback` 设置回调，在歌曲自然播放完毕时主动通知上层应用。 |

### ⚙️ 系统级特性

| 特性                 | 描述                                                         |
| -------------------- | ------------------------------------------------------------ |
| **高性能日志系统**   | 深度集成 `spdlog`，提供高性能异步日志。支持通过编译开关在Release版本中**彻底移除**日志代码，实现零性能开销。 |
| **现代C++20设计**    | 代码全部采用C++20标准，广泛使用智能指针、Pimpl模式、RAII等特性，保证内存安全和高可维护性。 |
| **为嵌入式平台而生** | 注重性能与资源占用的平衡，依赖 `FFmpeg` 等跨平台标准库，具备良好的可移植性。 |

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
  * **miniaudio**: [https://github.com/mackron/miniaudio](https://github.com/mackron/miniaudio)

FFmpeg 是一个领先的多媒体框架。`MusicEngine` 动态链接了其组件，这些组件在 **GNU Lesser General Public License (LGPL) version 3**  许可下使用
