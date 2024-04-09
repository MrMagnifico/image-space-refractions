#pragma once
#ifndef _COMMON_H_
#define _COMMON_H_

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/mesh.h>


struct HitInfo {
    glm::vec3 normal;
    glm::vec3 barycentricCoord;
    glm::vec2 texCoord;
    Material material;
};

struct Plane {
    float D = 0.0f;
    glm::vec3 normal { 0.0f, 1.0f, 0.0f };
};

struct AxisAlignedBox {
    glm::vec3 lower { 0.0f };
    glm::vec3 upper { 1.0f };
};

struct Sphere {
    glm::vec3 center { 0.0f };
    float radius = 1.0f;
    Material material;
};


#endif // _COMMON_H_
