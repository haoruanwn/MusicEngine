# SongManager 构建说明

## 一、环境要求

  * CMake (\>= 3.16)
  * Conan (\>= 2.0)
  * C++ 编译器 (支持 C++20)
  * Ninja (推荐的构建系统)

## 二、本地构建流程

我们以构建 `Release` 版本为例。

### 第一步：安装 Conan 依赖

此步骤会根据 `conanfile.txt` 的定义，下载或编译 `taglib` 依赖，并在指定的输出目录 (`build/linux-release`) 下生成 CMake 所需的工具链文件。

```bash
# 安装 Release 版本的依赖
conan install . --output-folder=build/linux-release --build=missing -s build_type=Release
```

> **提示**：如果需要构建 `Debug` 版本，只需修改对应的参数：
> `conan install . --output-folder=build/linux-debug --build=missing -s build_type=Debug`

### 第二步：配置 CMake 项目

此步骤使用 CMake 来配置项目，并指定使用上一步中 Conan 生成的工具链文件。这会在 `build/linux-release` 目录下生成 `Ninja` 构建系统的文件。

```bash
# 配置 Release 版本的项目
cmake -S . -B build/linux-release -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=build/linux-release/build/Release/generators/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release
```

> **提示**：构建 `Debug` 版本对应的命令为：
> `cmake -S . -B build/linux-debug -G Ninja -DCMAKE_TOOLCHAIN_FILE=build/linux-debug/build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug`

### 第三步：编译项目

配置完成后，执行编译命令来生成最终的可执行文件。

```bash
# 编译 Release 版本
cmake --build build/linux-release
```

> **提示**：编译 `Debug` 版本对应的命令为：
> `cmake --build build/linux-debug`

编译产物（例如 `basic_usage`）会出现在 `build/linux-release/examples/` 目录下。

## 三、交叉编译流程 (以 AArch64 为例)

交叉编译流程与本地构建非常相似，仅需在 `conan install` 步骤中指定目标平台的 `profile` 文件。

### 第一步：安装 AArch64 依赖 (Debug)

```bash
conan install . --output-folder=build/aarch64-debug --build=missing \
      --profile:host=./aarch64-linux-gnu \
      --profile:build=default \
      -s build_type=Debug
```

### 第二步：配置 CMake 项目 (AArch64 Debug)

```bash
cmake -S . -B build/aarch64-debug -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=build/aarch64-debug/build/Debug/generators/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug
```

### 第三步：编译项目 (AArch64 Debug)

```bash
cmake --build build/aarch64-debug
```
