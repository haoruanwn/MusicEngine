import os
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class SongManagerConan(ConanFile):
    name = "songmanager"
    version = "0.1.0"

    # 包的元数据
    license = "<Your project license>"
    author = "<haoruanwn> <2054190139@qq.com>"
    url = "<https://github.com/haoruanwn/SongManager>"
    description = "A library to manage and parse song metadata"
    topics = ("music", "metadata", "cpp")

    # 二进制包的配置
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # 源码导出
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "SongManagerConfig.cmake.in"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        # 使用标准的CMake目录布局，比如 build/Debug, build/Release
        cmake_layout(self)

    def requirements(self):
        # 在这里声明你的库的依赖！
        # Conan 会自动处理这些“传递依赖”
        self.requires("spdlog/1.15.3")
        self.requires("jsoncons/1.3.0")
        
    def generate(self):
        # 为下游消费者生成CMake查找脚本 (e.g., find_spdlog.cmake)
        deps = CMakeDeps(self)
        deps.generate()
        # 生成CMake工具链文件 (e.g., conan_toolchain.cmake)
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        # 封装了标准的CMake构建流程
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        # 将构建产物（库和头文件）打包
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        # 告诉下游消费者如何使用这个包
        self.cpp_info.libs = ["SongManager"] # 库名叫 libSongManager.a
        # 让消费者自动链接到我们库的依赖项
        self.cpp_info.requires = ["spdlog::spdlog", "jsoncons::jsoncons"]