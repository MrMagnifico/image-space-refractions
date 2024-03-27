#version 460

// Interpolated output data from vertex shader
layout(location = 0) in vec3 fragPos;       // World-space position
layout(location = 1) in vec3 fragNormal;    // World-space normal

// Output for on-screen color
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(abs(fragNormal), 1.0); // Output the normal as color
}