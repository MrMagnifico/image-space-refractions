#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_

enum class RenderOption {
    EnvironmentMap = 0,
    DepthFrontFace,
    DepthBackFace,
    NormalsFrontFace,
    NormalsBackFace,
    InnerObjectDistancesFrontFace,
    InnerObjectDistancesBackFace,
    Combined
};

struct Config {
    RenderOption currentRender { RenderOption::DepthFrontFace }; // The thing to be currently rendered

    float refractiveIndex { 0.15f };
};


#endif // _CONFIG_H_
