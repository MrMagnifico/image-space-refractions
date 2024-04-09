#include <framework/opengl_includes.h>      // Include glad before glfw3
#include <framework/trackball.h>
#include <framework/window.h>

#include <render/environment_map.h>
#include <render/mesh.h>
#include <render/refraction.h>
#include <ui/menu.h>
#include <utils/config.h>
#include <utils/constants.h>

#include <vector>


int main(int argc, char* argv[]) {
    // Init core objects
    Config config;
    Window window { "Image-Space Refractions", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46 };
    Trackball trackball { &window, glm::radians(50.0f) };
    Menu menu(config);
    RefractionRender refractionRender(config, window.getWindowSize());

    // Environment map
    EnvMapFilePaths envMapFilePaths = { .right  = utils::RESOURCES_PATH / "skybox" / "right.jpg",
                                        .left   = utils::RESOURCES_PATH / "skybox" / "left.jpg",
                                        .top    = utils::RESOURCES_PATH / "skybox" / "top.jpg",
                                        .bottom = utils::RESOURCES_PATH / "skybox" / "bottom.jpg",
                                        .front  = utils::RESOURCES_PATH / "skybox" / "front.jpg",
                                        .back   = utils::RESOURCES_PATH / "skybox" / "back.jpg" };
    EnvironmentMap environmentMap(envMapFilePaths);

    // Load resources
    std::vector<GPUMesh> subMeshes  = GPUMesh::loadMeshGPU(utils::RESOURCES_PATH / "dragon.obj");
    GPUMesh& mainMesh               = subMeshes[0];
    const Shader debugShader        = ShaderBuilder().addStage(GL_VERTEX_SHADER, utils::SHADERS_PATH / "deferred.vert")
                                                     .addStage(GL_FRAGMENT_SHADER, utils::SHADERS_PATH / "debug.frag").build();

    // Set GLFW key callback
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;
    });

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set default values for clearing framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    // Render loop
    while (!window.shouldClose()) {
        window.updateInput();

        // Set model matrix
        const glm::mat4 model = glm::mat4(1.0f);
        
        // Draw the requested option
        if (config.currentRender == RenderOption::EnvironmentMap) {
            environmentMap.render(trackball.viewMatrix(), trackball.projectionMatrix(), trackball.position());
        } else { refractionRender.draw(mainMesh, model, trackball.viewMatrix(), trackball.projectionMatrix()); }

        // Render UI
        menu.draw();

        // Present result to the screen.
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
