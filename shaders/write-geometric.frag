#version 460

// Input from vertex shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in float fragDistanceInner;

// Output for color attachments
layout(location = 0) out vec3 outColor;				// Normal texture
layout(location = 1) out float outDistanceInner;	// Distance to the nearest point on the interior of the mesh along normal (d_N in the paper)

void main() {
	outColor 			= fragNormal;
	outDistanceInner	= fragDistanceInner;
}
