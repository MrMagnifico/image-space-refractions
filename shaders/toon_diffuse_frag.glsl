#version 430

// Global variables for lighting calculations
layout (location = 1) uniform vec3 lightPos;
layout (location = 2) uniform vec3 lightColor;
layout (location = 3) uniform vec3 objectDiffuse;
layout (location = 4) uniform int toonIntervals;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

void main() {
    // https://en.wikipedia.org/wiki/Lambertian_reflectance
    vec3 lightDir           = normalize(lightPos - fragPos);
    vec3 lambertianDiffuse  = dot(lightDir, fragNormal) * lightColor * objectDiffuse;

    // TODO: Discretize to closest bound within each interval
    outColor            = vec4(0.0, 0.0, 0.0, 1.0);
    float intervalStep  = 1.0 / toonIntervals;
    float lowerBound    = 0.0;
    float upperBound    = intervalStep;
    for (int interval = 0; interval < toonIntervals; interval++, lowerBound += intervalStep, upperBound += intervalStep) {
        if (lowerBound < lambertianDiffuse.r && lambertianDiffuse.r < upperBound) { outColor.r = upperBound; }
        if (lowerBound < lambertianDiffuse.g && lambertianDiffuse.g < upperBound) { outColor.g = upperBound; }
        if (lowerBound < lambertianDiffuse.b && lambertianDiffuse.b < upperBound) { outColor.b = upperBound; }
    }
}
