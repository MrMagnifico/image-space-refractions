#version 460

// Texture to render to screen
layout(location = 0) uniform sampler2D texSampler;

// Input from vertex shader
layout(location = 0) in vec3 fragPos;       // Screen-space coordinates
layout(location = 1) in vec3 fragNormal;    // World-space normal

// TODO: Add output for world-space normal texture

void main() {
    gl_FragDepth = fragPos.z;
}
