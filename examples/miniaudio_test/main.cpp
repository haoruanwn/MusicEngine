#include <cmath>
#include <iostream>
#include "spdlog/spdlog.h" 
#include "spdlog/sinks/stdout_color_sinks.h"


#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 音频回调函数 
void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
    float *pPhase = (float *) pDevice->pUserData;
    float *pOutputF32 = (float *) pOutput;
    const float amplitude = 0.2f;
    const float frequency = 440.0f;
    const float phaseIncrement = (2.0f * M_PI * frequency) / pDevice->sampleRate;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        float sample = amplitude * sin(*pPhase);
        pOutputF32[i * 2 + 0] = sample; // 左声道
        pOutputF32[i * 2 + 1] = sample; // 右声道
        *pPhase += phaseIncrement;
        if (*pPhase >= 2.0f * M_PI) {
            *pPhase -= 2.0f * M_PI;
        }
    }
    (void) pInput;
}

int main() {

    auto console_logger = spdlog::stdout_color_mt("audio_test");
    spdlog::set_default_logger(console_logger);
    spdlog::set_level(spdlog::level::info); // 设置全局日志级别

    ma_device_config deviceConfig;
    ma_device device;
    float sinePhase = 0.0f;

    // 1. 初始化设备配置
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    spdlog::info("Using default playback device.");

    // 2. 配置我们关心的参数
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 48000;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &sinePhase;

    // 3. 初始化设备
    ma_result result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        // 使用 spdlog::error 输出错误信息，使用 {} 作为占位符
        spdlog::error("Failed to initialize audio device. Error code: {}", static_cast<int>(result));
        return -1;
    }

    // 4. 启动设备
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        spdlog::error("Failed to start audio device. Error code: {}", static_cast<int>(result));
        ma_device_uninit(&device);
        return -1;
    }

    spdlog::info("Device started. Playing a 440Hz tone.");
    spdlog::info("Press Enter to quit...");
    std::cin.get(); 

    spdlog::info("Stopping device...");
    // 5. 清理资源
    ma_device_uninit(&device);
    
    spdlog::info("Device stopped. Exiting.");
    return 0;
}