#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

enum class RenderOption {
    DepthFrontFace = 0,
    DepthBackFace,
    NormalsFrontFace,
    NormalsBackFace,
    InnerObjectDistancesFrontFace,
    InnerObjectDistancesBackFace,
    Combined
};

struct Config {
    // Refraction rendering
    RenderOption currentRender  { RenderOption::Combined }; // The thing to be currently rendered
    bool showEnvironmentMap     { true };
    float refractiveIndexRatio  { 1.1f };
    glm::vec3 transparency      { 1.0f };

    // Inner object distance ray-tracing
    bool useBVH                 { true };
};


#endif // _CONFIG_H_
