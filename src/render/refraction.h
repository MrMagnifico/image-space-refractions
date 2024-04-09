#pragma once
#ifndef _REFRACTION_H_
#define _REFRACTION_H_

#include <framework/opengl_includes.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
DISABLE_WARNINGS_POP()

#include <render/mesh.h>
#include <utils/config.h>


class RefractionRender {
public:
    RefractionRender(Config& config, glm::ivec2 windowDims);
    ~RefractionRender();

    void draw(const GPUMesh& mesh, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);

private:
    void initShaders();
    void initTexturesAndFramebuffers();
    void renderGeometry(const GPUMesh& mesh, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void renderGeometrySingle(const GPUMesh& mesh, const glm::mat4& model, const glm::mat3& normalModel, const glm::mat4& mvp);
    void drawQuad(GLuint texture);

    Config& m_config;

    glm::ivec2 m_windowDims;
    Shader m_renderGeometry, m_screenQuad;

    GLuint m_depthTexFront, m_depthTexBack;
    GLuint m_normalsTexFront, m_normalsTexBack;
    GLuint m_framebufferFront, m_framebufferBack;
};


#endif // _REFRACTION_H_
