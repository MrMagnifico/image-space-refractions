#include "refraction.h"

DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

#include <framework/trackball.h>

#include <utils/constants.h>
#include <utils/render_utils.hpp>
#include <utils/magic_enum.hpp>

#include <array>


RefractionRender::RefractionRender(Config& config, glm::ivec2 windowDims)
    : m_config(config)
    , m_windowDims(windowDims) {
    initShaders();
    initTexturesAndFramebuffers();
}

RefractionRender::~RefractionRender() {
    std::array<GLuint, 2> framebuffers  = { m_framebufferFront, m_framebufferBack };
    glDeleteFramebuffers(framebuffers.size(), framebuffers.data());
    std::array<GLuint, 6> textures      = { m_normalsTexFront, m_normalsTexBack,
                                            m_depthTexFront, m_depthTexBack,
                                            m_innerDistTexFront, m_innerDistTexBack };
    glDeleteTextures(textures.size(), textures.data());
}

void RefractionRender::initShaders() {
    m_renderGeometry    = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "write-geometric.vert")
                                         .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "write-geometric.frag").build();
    m_screenQuad        = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "screen-quad.vert")
                                         .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "screen-quad.frag").build();
    m_renderCombined    = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "refract-render.vert")
                                         .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "refract-render.frag").build();
}

void RefractionRender::initTexturesAndFramebuffers() {
    // Define specifications to be reused
    std::array<GLint, 4> swizzleAllAccessRed = {GL_RED, GL_RED, GL_RED, GL_RED};        // RGBA accesses will all access the R channel
    std::array<GLuint, 2> attachments { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };   // Framebuffer will render to two color attachments

    // Depth textures
    std::array<GLuint*, 2> depthTexPtrs = { &m_depthTexFront, &m_depthTexBack };
    for (GLuint* texPtr : depthTexPtrs) {
        glCreateTextures(GL_TEXTURE_2D, 1, texPtr);
        glTextureStorage2D(*texPtr, 1, GL_DEPTH_COMPONENT32F, m_windowDims.x, m_windowDims.y);
        glTextureParameteri(*texPtr, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(*texPtr, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteriv(*texPtr, GL_TEXTURE_SWIZZLE_RGBA, swizzleAllAccessRed.data());
    }

    // Normal textures
    std::array<GLuint*, 2> normalTexPtrs = { &m_normalsTexFront, &m_normalsTexBack };
    for (GLuint* texPtr : normalTexPtrs) {
        glCreateTextures(GL_TEXTURE_2D, 1, texPtr);
        glTextureStorage2D(*texPtr, 1, GL_RGB16F, m_windowDims.x, m_windowDims.y);
        glTextureParameteri(*texPtr, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(*texPtr, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // Inner distance textures
    std::array<GLuint*, 2> innerDistTexPtrs = { &m_innerDistTexFront, &m_innerDistTexBack };
    for (GLuint* texPtr : innerDistTexPtrs) {
        glCreateTextures(GL_TEXTURE_2D, 1, texPtr);
        glTextureStorage2D(*texPtr, 1, GL_R32F, m_windowDims.x, m_windowDims.y);
        glTextureParameteri(*texPtr, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(*texPtr, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteriv(*texPtr, GL_TEXTURE_SWIZZLE_RGBA, swizzleAllAccessRed.data());
    }

    // Front face framebuffer. Create and attach textures as render targets
    glCreateFramebuffers(1, &m_framebufferFront);
    glNamedFramebufferTexture(m_framebufferFront, GL_DEPTH_ATTACHMENT, m_depthTexFront, 0);
    glNamedFramebufferTexture(m_framebufferFront, GL_COLOR_ATTACHMENT0, m_normalsTexFront, 0);
    glNamedFramebufferTexture(m_framebufferFront, GL_COLOR_ATTACHMENT1, m_innerDistTexFront, 0);
    glNamedFramebufferDrawBuffers(m_framebufferFront, attachments.size(), attachments.data());

    // Back face framebuffer. Create and attach textures as render targets
    glCreateFramebuffers(1, &m_framebufferBack);
    glNamedFramebufferTexture(m_framebufferBack, GL_DEPTH_ATTACHMENT, m_depthTexBack, 0);
    glNamedFramebufferTexture(m_framebufferBack, GL_COLOR_ATTACHMENT0, m_normalsTexBack, 0);
    glNamedFramebufferTexture(m_framebufferBack, GL_COLOR_ATTACHMENT1, m_innerDistTexBack, 0);
    glNamedFramebufferDrawBuffers(m_framebufferBack, attachments.size(), attachments.data());
}

void RefractionRender::draw(const GPUMesh& mesh,
                            const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                            const glm::vec3& cameraPosition, const GLuint environmentMapTex) {
    // Render geometry info so we can draw whatever we want
    renderGeometry(mesh, model, view, projection);

    // Use rendered data to display the actual requested thing
    switch (m_config.currentRender) {
        case RenderOption::DepthFrontFace: {
            drawQuad(m_depthTexFront);
        } break;
        case RenderOption::DepthBackFace: {
            drawQuad(m_depthTexBack);
        } break;
        case RenderOption::NormalsFrontFace: {
            drawQuad(m_normalsTexFront);
        } break;
        case RenderOption::NormalsBackFace: {
            drawQuad(m_normalsTexBack);
        } break;
        case RenderOption::InnerObjectDistancesFrontFace: {
            drawQuad(m_innerDistTexFront);
        } break;
        case RenderOption::InnerObjectDistancesBackFace: {
            drawQuad(m_innerDistTexBack);
        } break;
        case RenderOption::Combined: {
            renderCombined(mesh, model, view, projection, cameraPosition, environmentMapTex);
        } break;
    }
}

void RefractionRender::renderGeometry(const GPUMesh& mesh, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    // Get original depth function
    GLint originalDepthFunction;
    glGetIntegerv(GL_DEPTH_FUNC, &originalDepthFunction);

    // Set viewport and compute needed matrices
    glViewport(0, 0, m_windowDims.x, m_windowDims.y);
    const glm::mat3 normalModel = glm::inverseTranspose(glm::mat3(model));
    const glm::mat4 mvp         = projection * view * model;

    // Render front faces
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferFront);
    renderGeometrySingle(mesh, model, normalModel, mvp);

    // Render back faces
    glClearDepth(0.0f);
    glDepthFunc(GL_GREATER);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferBack);
    renderGeometrySingle(mesh, model, normalModel, mvp);

    // Restore original OpenGL state
    glClearDepth(1.0f);
    glDepthFunc(originalDepthFunction);
}

void RefractionRender::renderGeometrySingle(const GPUMesh& mesh,
                                            const glm::mat4& model, const glm::mat3& normalModel, const glm::mat4& mvp) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_renderGeometry.bind();
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModel));
    mesh.draw(m_renderGeometry);
}

void RefractionRender::renderCombined(const GPUMesh& mesh,
                                      const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                                      const glm::vec3& cameraPosition, const GLuint environmentMapTex) {
    // Set screen buffer, clear it, set viewport, and bind combined render shader program
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_windowDims.x, m_windowDims.y);
    m_renderCombined.bind();
    
    // Uniforms: Transformation matrices
    const glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(model));

    // Uniforms: Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depthTexFront);
    glUniform1i(2, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depthTexBack);
    glUniform1i(3, 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_normalsTexFront);
    glUniform1i(4, 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_normalsTexBack);
    glUniform1i(5, 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_innerDistTexFront);
    glUniform1i(6, 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMapTex);
    glUniform1i(7, 5);

    // Uniforms: Miscellaneous
    glUniform3fv(8, 1, glm::value_ptr(cameraPosition));
    glUniform1f(9, m_config.refractiveIndexRatio);
    glUniform1f(10, Trackball::NEAR_PLANE);
    glUniform1f(11, Trackball::FAR_PLANE);

    // Draw the mesh
    mesh.draw(m_renderCombined);
}

void RefractionRender::drawQuad(GLuint texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_windowDims.x, m_windowDims.y);
    m_screenQuad.bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(0, 0);
    utils::renderQuad();
}
