# CMake toolchain file for arm-rockchip830-linux-uclibcgnueabihf

# 说明：虽然提供了设置工具链的cmake文件，但是我试着在CMakeLists中include它，并不能成功，我是在clion的工具链选择中指定的交叉编译器。
# Target system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross compiler paths
# set(CROSS_COMPILER_PREFIX /home/hao/tools/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# 设置查找路径规则
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# set(CMAKE_C_COMPILER ${CROSS_COMPILER_PREFIX}gcc)
# set(CMAKE_CXX_COMPILER ${CROSS_COMPILER_PREFIX}g++)


#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Compiler Flags (Project-Wide) ---
#add_compile_options(
#        -O3 -g0
#        # C Specific
#        $<$<COMPILE_LANGUAGE:C>:-std=c99>
#        # C++ Specific
#        # $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> # 例如
#
#        # Common Warnings
#        -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wextra
#        -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing
#        -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter
#        -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess
#        -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default
#        -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers
#        -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes
#        -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits
#        -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value
#)

# Set the sysroot if necessary (often needed for includes and libraries)
# Example: set(CMAKE_SYSROOT /home/hao/luckfox-pico/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/arm-rockchip830-linux-uclibcgnueabihf/sysroot)
# If your toolchain handles sysroot implicitly, you might not need this.
# Adjust the path above if your sysroot is located elsewhere within the toolchain directory.
#set(TSPI_SYSROOT "$ENV{HOME}/tspi_ubuntu_sysroot")
#set(CMAKE_SYSROOT ${TSPI_SYSROOT})
# set(CMAKE_SYSROOT /home/hao/luckfox-pico/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/arm-rockchip830-linux-uclibcgnueabihf/sysroot)

# Configure find commands
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)