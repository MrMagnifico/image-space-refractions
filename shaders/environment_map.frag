#version 460

layout(location = 1) uniform samplerCube texSampler;

layout(location = 0) in vec3 texCoords;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, texCoords);
}
