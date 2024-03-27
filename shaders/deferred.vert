#version 460

// Uniforms
layout(location = 0) uniform mat4 mvp;                          // Full MVP matrix
layout(location = 1) uniform mat4 model;                        // Model matrix only
layout(location = 2) uniform mat3 normalModel;                  // Normals should be transformed differently than positions (https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html)
layout(location = 3) uniform bool outputScreenSpace = false;    // If true, the position pass to the fragment shader will be in screen space instead of world space

// Per-vertex attributes
layout(location = 0) in vec3 pos;       // Model-space position
layout(location = 1) in vec3 normal;    // Model-space normal

// Data to pass to fragment shader
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

void main() {
	// Transform 3D position into on-screen position
    gl_Position = mvp * vec4(pos, 1.0);

    // Pass world-space position and normal through to fragment shader
    fragPos     = outputScreenSpace ? gl_Position.xyz : (model * vec4(pos, 1.0)).xyz;
    fragNormal  = normalModel * normal;
}
