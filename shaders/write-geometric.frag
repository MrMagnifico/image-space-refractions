#version 460

// Input from fragment shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;

// Output for color attachments
layout(location = 0) out vec3 outColor; // We will store the normal here

void main() {
	outColor = fragNormal;
}
