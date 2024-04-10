#version 460

// Uniforms
layout(location = 0) uniform mat4 mvp;      // Full MVP matrix
layout(location = 1) uniform mat4 model;    // Model matrix only

// Per-vertex attributes
layout(location = 0) in vec3 pos;               // Model-space position
layout(location = 1) in vec3 normal;            // Model-space normal
layout(location = 2) in vec2 texCoord;          // Texture coordinates
layout(location = 3) in float distanceInner;    // Distance to the nearest point on the interior of the mesh along normal (d_N in the paper)

// Data to pass to fragment shader
layout(location = 0) out vec3 fragPosWorld;     // World-space fragment position
layout(location = 1) out vec3 fragPosScreen;    // Screen-space fragment position (NDC space, i.e. [-1, 1])

void main() {
    vec4 screenPos  = mvp * vec4(pos, 1.0);         // Transform 3D position into on-screen position
    gl_Position     = screenPos;
    fragPosScreen   = screenPos.xyz /= screenPos.w;
    fragPosWorld    = (model * vec4(pos, 1.0)).xyz;
}
