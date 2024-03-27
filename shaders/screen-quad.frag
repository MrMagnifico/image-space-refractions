#version 460

// Texture to render to screen
layout(location = 0) uniform sampler2D texSampler;

// Screen-space coordinates
layout(location = 0) in vec2 bufferCoords;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

void main() {
    vec2 texCoords  = bufferCoords * 0.5 + 0.5; // Transform from NDC coordinates [-1, 1] to normalised UV coordinates [0, 1]
    outColor        = texture(texSampler, texCoords);
}
