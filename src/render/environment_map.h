#pragma once
#ifndef _ENVIRONMENT_MAP_H_
#define _ENVIRONMENT_MAP_H_

#include <framework/opengl_includes.h>
#include <framework/shader.h>
#include <framework/disable_all_warnings.h> // Disable compiler warnings in third-party code (which we cannot change)
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
DISABLE_WARNINGS_POP()

#include <filesystem>

struct EnvMapFilePaths {
    std::filesystem::path right, left,
                          top, bottom,
                          front, back;
};

class EnvironmentMap {
public:
    EnvironmentMap(const EnvMapFilePaths& texPaths);
    ~EnvironmentMap();

    void render(const glm::mat4& projection, const glm::vec3 cameraForward, const glm::vec3& cameraUp);

    GLuint getTexId() { return m_cubemapTex; }

private:
    static constexpr GLsizei NUM_CUBE_TRIANGLES = 12;

    GLuint m_cubemapTex;
    GLuint m_cubeVAO;
    GLuint m_cubeVBO;
    Shader m_renderCubeMap;
    
    void createCubemapTex(const EnvMapFilePaths& texPaths);
    void createCubeBuffers();
    void compileRenderShader();
};


#endif // _ENVIRONMENT_MAP_H_
