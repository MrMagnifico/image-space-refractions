#include <framework/opengl_includes.h>      // Include glad before glfw3
#include <framework/disable_all_warnings.h> // Disable compiler warnings in third-party code (which we cannot change)
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/trackball.h>
#include <framework/window.h>

#include <utils/constants.h>
#include <mesh.h>

#include <vector>


int main(int argc, char* argv[]) {
    // Init core objects
    Window window { "Shading", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL45 };
    Trackball trackball { &window, glm::radians(50.0f) };

    // Load resources
    std::vector<GPUMesh> subMeshes  = GPUMesh::loadMeshGPU(utils::RESOURCES_PATH / "dragon.obj");
    GPUMesh& mainMesh               = subMeshes[0];
    const Shader debugShader        = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "vertex.glsl")
                                                     .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "debug_frag.glsl").build();

    // Set GLFW key callback
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;
    });

    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!window.shouldClose()) {
        window.updateInput();

        // Clear the framebuffer to black and depth to maximum value (ranges from [-1.0 to +1.0]).
        glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set MVP matrix.
        const glm::mat4 model { 1.0f };
        const glm::mat4 view        = trackball.viewMatrix();
        const glm::mat4 projection  = trackball.projectionMatrix();
        const glm::mat4 mvp         = projection * view * model;

        // Bind debug shader, push MVP uniform, and render
        debugShader.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
        mainMesh.draw(debugShader);

        // Present result to the screen.
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
