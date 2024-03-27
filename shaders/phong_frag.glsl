#version 430

// Global variables for lighting calculations
layout (location = 1) uniform vec3 lightPos;
layout (location = 2) uniform vec3 lightColor;
layout (location = 3) uniform vec3 cameraPos;
layout (location = 4) uniform vec3 objectSpecular;
layout (location = 5) uniform float objectShininess;


// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

void main() {
    // https://en.wikipedia.org/wiki/Phong_reflection_model
    vec3 lightToSurface     = normalize(fragPos - lightPos);
    vec3 surfaceToCamera    = normalize(cameraPos - fragPos);
    vec3 reflection         = reflect(lightToSurface, fragNormal);

    float lightNormalDot    = dot(-lightToSurface, fragNormal);
    float reflectionViewDot = dot(reflection, surfaceToCamera);
    outColor                = lightNormalDot > 0.0 && reflectionViewDot > 0.0 ?
                              vec4(objectSpecular * pow(reflectionViewDot, objectShininess) * lightColor, 1.0) :
                              vec4(0.0, 0.0, 0.0, 1.0);
}
