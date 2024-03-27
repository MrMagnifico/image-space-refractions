#version 460

// Texture to render to screen
layout(location = 0) uniform sampler2D texSampler;

// Screen-space coordinates
layout(location = 0) in vec2 bufferCoords;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, bufferCoords);
}
