#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_

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
    float refractiveIndexRatio  { 1.0f };

    // Inner object distance ray-tracing
    bool useBVH                 { true };
};


#endif // _CONFIG_H_
