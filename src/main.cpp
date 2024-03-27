#include <framework/opengl_includes.h>      // Include glad before glfw3
#include <framework/disable_all_warnings.h> // Disable compiler warnings in third-party code (which we cannot change)
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/trackball.h>
#include <framework/window.h>

#include <utils/constants.h>
#include <utils/render_utils.hpp>
#include <mesh.h>

#include <array>
#include <vector>


int main(int argc, char* argv[]) {
    // Init core objects
    Window window { "Shading", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46 };
    Trackball trackball { &window, glm::radians(50.0f) };

    // Load resources
    std::vector<GPUMesh> subMeshes  = GPUMesh::loadMeshGPU(utils::RESOURCES_PATH / "dragon.obj");
    GPUMesh& mainMesh               = subMeshes[0];
    const Shader debugShader        = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "deferred.vert")
                                                     .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "debug.frag").build();
    const Shader geomInfoWrite      = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "deferred.vert")
                                                     .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "write-normal.frag").build();
    const Shader screenQuad         = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "screen-quad.vert")
                                                     .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "screen-quad.frag").build();

    // Set GLFW key callback
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;
    });

    // Create depth texture
    std::array<GLint, 4> swizzleAllAccessRed = {GL_RED, GL_RED, GL_RED, GL_RED};
    GLuint texDepth;
    glCreateTextures(GL_TEXTURE_2D, 1, &texDepth);
    glTextureStorage2D(texDepth, 1, GL_DEPTH_COMPONENT32F, window.getWindowSize().x, window.getWindowSize().y);
    glTextureParameteri(texDepth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texDepth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteriv(texDepth, GL_TEXTURE_SWIZZLE_RGBA, swizzleAllAccessRed.data());

    // Create normal texture
    GLuint texNormal;
    glCreateTextures(GL_TEXTURE_2D, 1, &texNormal);
    glTextureStorage2D(texNormal, 1, GL_RGB16F, window.getWindowSize().x, window.getWindowSize().y);
    glTextureParameteri(texNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(texNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach depth and normal textures to framebuffer
    GLuint geomInfoFramebuffer;
    glCreateFramebuffers(1, &geomInfoFramebuffer);
    glNamedFramebufferTexture(geomInfoFramebuffer, GL_DEPTH_ATTACHMENT, texDepth, 0);
    glNamedFramebufferTexture(geomInfoFramebuffer, GL_COLOR_ATTACHMENT0, texNormal, 0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set default values for clearing framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    // Render loop
    while (!window.shouldClose()) {
        window.updateInput();

        // Set model (incl. normal) and MVP matrices
        const glm::mat4 model       = glm::mat4(1.0f);
        const glm::mat3 normalModel = glm::inverseTranspose(glm::mat3(model));
        const glm::mat4 mvp         = trackball.projectionMatrix() * trackball.viewMatrix() * model;

        // Render to geometry info textures
        glBindFramebuffer(GL_FRAMEBUFFER, geomInfoFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        geomInfoWrite.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normalModel));
        mainMesh.draw(geomInfoWrite);

        // Draw the normal texture to the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        screenQuad.bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texNormal);
        glUniform1i(0, 0);
        utils::renderQuad();

        // Present result to the screen.
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
