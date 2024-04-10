#version 460

// Transformation matrices
layout(location = 0) uniform mat4 mvp;      // Full MVP matrix
layout(location = 1) uniform mat4 model;    // Model matrix only

// Textures
layout(location = 2) uniform sampler2D frontDepth;
layout(location = 3) uniform sampler2D backDepth;
layout(location = 4) uniform sampler2D frontNormals;
layout(location = 5) uniform sampler2D backNormals;
layout(location = 6) uniform sampler2D innerDistance;
layout(location = 7) uniform samplerCube environmentMap;

// Miscellaneous
layout(location = 8) uniform vec3 cameraPosition;
layout(location = 9) uniform float refractiveIndexRatio;
layout(location = 10) uniform float nearPlaneDist;
layout(location = 11) uniform float farPlaneDist;

// Input from vertex shader
layout(location = 0) in vec3 fragPosWorld;  // World-space fragment position
layout(location = 1) in vec3 fragPosScreen; // Screen-space fragment position (NDC space, i.e. [-1, 1])

// Output for color attachments
layout(location = 0) out vec4 outColor; // On-screen color

void main() {
    vec2 texCoords = fragPosScreen.xy * 0.5 + 0.5;

    // Compute exterior entry angle
	vec3 fragToCamera   = normalize(cameraPosition - fragPosWorld);
    vec3 normalFront    = texture(frontNormals, texCoords).xyz;
    float cosExterior   = dot(fragToCamera, normalFront); // Theta_i in paper

    // Compute interior entry angle
    vec3 cameraToFrag           = -fragToCamera;
    vec3 refractionDirection    = refract(cameraToFrag, normalFront, refractiveIndexRatio);
    vec3 normalFrontInverse     = -normalFront;
    float cosInterior           = dot(refractionDirection, normalFrontInverse); // Theta_t in paper

    // Compute exit point
    // Depth is in range [0, 1]
    // 0 corresponds to near plane distance
    // 1 corresponds to far plane distance
    // We map back to obtain world-space distance
    float interPlaneDist                = farPlaneDist - nearPlaneDist;
    float unrefractedDistance           = interPlaneDist * (texture(backDepth, texCoords).x - texture(frontDepth, texCoords).x) // d_V in paper
                                          + nearPlaneDist;
    float distanceInner                 = texture(innerDistance, texCoords).x;                                                  // d_N in paper
    float angleRatio                    = acos(cosInterior) / acos(cosExterior);
    float approximateRefractionDistance = (angleRatio * unrefractedDistance) + ((1.0 - angleRatio) * distanceInner);
    vec3 exitPointWorld                 = fragPosWorld + (approximateRefractionDistance * refractionDirection);

    // Compute exit direction
    vec4 exitPointScreen            = mvp * vec4(exitPointWorld, 1.0);
    vec2 exitPointTexCoords         = (exitPointScreen.xy / exitPointScreen.w) * 0.5 + 0.5;
    vec3 exitNormal                 = -texture(backNormals, exitPointTexCoords).xyz;                        // Refract expects normal defining a hemisphere that the incident direction is in
    vec3 exitRefractionDirection    = refract(refractionDirection, exitNormal, 1.0 / refractiveIndexRatio); // The entry and exit media have been flipped, so this second refraction uses the repicrocal of their ratio
    
    // Compute final color
    // TODO: Remove when environment map is fixed
    exitRefractionDirection.z   = -exitRefractionDirection.z; // Front and back are reversed for some reason. Probably a bug with the environment map
    outColor                    = texture(environmentMap, exitRefractionDirection);
}
