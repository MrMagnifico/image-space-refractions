#pragma once
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <framework/opengl_includes.h>

#include <filesystem>

namespace utils {
    // OpenGL constants
    constexpr GLuint INVALID = 0xFFFFFFFF;
    
    // Resource paths
    const std::filesystem::path CACHE_PATH      = CACHE_DIR;
    const std::filesystem::path RESOURCES_PATH  = RESOURCES_DIR;
    const std::filesystem::path SHADERS_PATH    = SHADERS_DIR;

    // Initial window size config
    constexpr int32_t WIDTH     = 1280;
    constexpr int32_t HEIGHT    = 720;

    // Numerical constants
    constexpr float ZERO_EPSILON        = 1e-5f;
    constexpr float INTERIOR_RAY_OFFSET = 1e-3f;
}

#endif 
