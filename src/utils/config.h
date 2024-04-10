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
    RenderOption currentRender  { RenderOption::Combined }; // The thing to be currently rendered
    bool showEnvironmentMap     { false };
    float refractiveIndexRatio  { 0.15f };

    bool useBVH                 { true };
};


#endif // _CONFIG_H_
