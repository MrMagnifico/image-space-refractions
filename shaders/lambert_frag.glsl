#version 430

// Global variables for lighting calculations
layout (location = 1) uniform vec3 lightPos;
layout (location = 2) uniform vec3 lightColor;
layout (location = 3) uniform vec3 objectDiffuse;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

void main() {
    // https://en.wikipedia.org/wiki/Lambertian_reflectance
    vec3 lightDir   = normalize(lightPos - fragPos);
    outColor        = vec4(dot(lightDir, fragNormal) * lightColor * objectDiffuse, 1.0);
}
