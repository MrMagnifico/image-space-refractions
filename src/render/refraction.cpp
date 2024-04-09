#include "refraction.h"

DISABLE_WARNINGS_PUSH()
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()

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
    std::array<GLuint, 2> framebuffers = { m_framebufferFront, m_framebufferBack };
    glDeleteFramebuffers(2, framebuffers.data());
    std::array<GLuint, 4> textures = { m_normalsTexFront, m_normalsTexBack, m_depthTexFront, m_depthTexBack };
    glDeleteTextures(4, textures.data());
}

void RefractionRender::initShaders() {
    m_renderGeometry    = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "deferred.vert")
                                         .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "write-normal.frag").build();
    m_screenQuad        = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "screen-quad.vert")
                                         .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "screen-quad.frag").build();
}

void RefractionRender::initTexturesAndFramebuffers() {
    // Defines that RGBA accesses will all access the R channel. Used for depth textures.
    std::array<GLint, 4> swizzleAllAccessRed = {GL_RED, GL_RED, GL_RED, GL_RED};

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

    // Front face framebuffer. Create and attach textures as render targets
    glCreateFramebuffers(1, &m_framebufferFront);
    glNamedFramebufferTexture(m_framebufferFront, GL_DEPTH_ATTACHMENT, m_depthTexFront, 0);
    glNamedFramebufferTexture(m_framebufferFront, GL_COLOR_ATTACHMENT0, m_normalsTexFront, 0);

    // Back face framebuffer. Create and attach textures as render targets
    glCreateFramebuffers(1, &m_framebufferBack);
    glNamedFramebufferTexture(m_framebufferBack, GL_DEPTH_ATTACHMENT, m_depthTexBack, 0);
    glNamedFramebufferTexture(m_framebufferBack, GL_COLOR_ATTACHMENT0, m_normalsTexBack, 0);
}

void RefractionRender::draw(const GPUMesh& mesh, 
                            const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    // Render geometry info so we can draw whatever we want
    renderGeometry(mesh, model, view, projection);

    // Use rendered data to display the actual requested thing
    switch (m_config.currentRender) {
        case RenderOption::EnvironmentMap: {
            throw std::runtime_error("Requested refraction renderer to draw environment map");
        } break;
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
        case RenderOption::InnerObjectDistancesFrontFace:
        case RenderOption::InnerObjectDistancesBackFace:
        case RenderOption::Combined:
            throw std::runtime_error(std::format("{} not implemented!", magic_enum::enum_name(m_config.currentRender)));
            break;
    }
}

void RefractionRender::renderGeometry(const GPUMesh& mesh,
                                      const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
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
