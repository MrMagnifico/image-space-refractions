#version 460

layout(location = 0) uniform mat4 cubeTransform;

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 texCoords;

void main() {
    vec2 screenSpaceXY  = (cubeTransform * vec4(position, 1.0)).xy;
    gl_Position         = vec4(screenSpaceXY, 1.0, 1.0);
    texCoords           = position;
}
