#pragma once
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <filesystem>

namespace utils {
    // Resource paths
    const std::filesystem::path RESOURCES_PATH  = RESOURCES_DIR;
    const std::filesystem::path SHADERS_PATH    = SHADERS_DIR;

    // Initial window size config
    constexpr int32_t WIDTH     = 1024;
    constexpr int32_t HEIGHT    = 600;
}

#endif 
