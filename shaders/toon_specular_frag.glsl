#version 430

// Global variables for lighting calculations
layout (location = 1) uniform vec3 lightPos;
layout (location = 2) uniform vec3 lightColor;
layout (location = 3) uniform vec3 cameraPos;
layout (location = 4) uniform vec3 objectSpecular;
layout (location = 5) uniform float objectShininess;
layout (location = 6) uniform float specularThreshold;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

void main() {
    // https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model
    vec3 surfaceToLight     = normalize(lightPos - fragPos);
    vec3 surfaceToCamera    = normalize(cameraPos - fragPos);
    vec3 vectorsSum         = surfaceToLight + surfaceToCamera;
    vec3 halfway            = vectorsSum / length(vectorsSum);

    float lightNormalDot    = dot(surfaceToLight, fragNormal);
    float normalHalfwayDot  = dot(fragNormal, halfway);
    vec3 phongSpecular      = lightNormalDot > 0.0 && normalHalfwayDot > 0.0 ?
                              objectSpecular * pow(normalHalfwayDot, objectShininess) * lightColor :
                              vec3(0.0, 0.0, 0.0);

    outColor = length(phongSpecular) >= specularThreshold ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}
