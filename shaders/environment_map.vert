#version 460

layout(location = 0) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 texCoords;

void main() {
    gl_Position = viewProjection * vec4(position, 1.0);
    texCoords   = position;
}
