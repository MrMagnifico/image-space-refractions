#version 430

// Global variables for lighting calculations
layout (location = 1) uniform vec3 lightPos;
layout (location = 2) uniform vec3 lightColor;
layout (location = 3) uniform vec3 cameraPos;
layout (location = 4) uniform vec3 objectDiffuse;
layout (location = 5) uniform vec3 objectSpecular;
layout (location = 6) uniform float objectShininess;
layout (location = 7) uniform sampler2D toonTexture;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

float uniformMap(float value, float domainLower, float domainUpper, float rangeLower, float rangeUpper) {
    return ((value - domainLower) / (rangeUpper - rangeLower)) + rangeLower;
}

void main() {
    // https://en.wikipedia.org/wiki/Lambertian_reflectance
    vec3 lightDir           = normalize(lightPos - fragPos);
    vec3 lambertianDiffuse  = dot(lightDir, fragNormal) * lightColor * objectDiffuse;

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

    // Sample texture using Phong magnitude and distance normalised to fixed range
    float phongBrightness   = clamp(length(lambertianDiffuse + phongSpecular), 0.0, 1.0);
    float mappedDistance    = uniformMap(length(fragPos), 0.0, 100.0, 0.0, 1.0);
    outColor                = texture(toonTexture, vec2(phongBrightness, mappedDistance));
}
