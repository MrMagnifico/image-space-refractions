#include <framework/opengl_includes.h>
#include <framework/trackball.h>
#include <framework/window.h>

#include <render/environment_map.h>
#include <render/mesh_manager.h>
#include <render/refraction.h>
#include <ui/menu.h>
#include <utils/config.h>
#include <utils/constants.h>

int main(int argc, char* argv[]) {
    // Init core objects
    Config config;
    Window window { "Interactive Refraction", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46 };
    Trackball trackball { &window, glm::radians(50.0f) };
    MeshManager meshManager(config, utils::RESOURCES_PATH / "dragon.obj");
    Menu menu(config, meshManager);
    RefractionRender refractionRender(config, window.getWindowSize());

    // Environment map
    constexpr char envMapFolder[]   = "Skansen";
    EnvMapFilePaths envMapFilePaths = { .right  = utils::RESOURCES_PATH / envMapFolder / "posx.jpg",
                                        .left   = utils::RESOURCES_PATH / envMapFolder / "negx.jpg",
                                        .top    = utils::RESOURCES_PATH / envMapFolder / "posy.jpg",
                                        .bottom = utils::RESOURCES_PATH / envMapFolder / "negy.jpg",
                                        .front  = utils::RESOURCES_PATH / envMapFolder / "posz.jpg",
                                        .back   = utils::RESOURCES_PATH / envMapFolder / "negz.jpg" };
    EnvironmentMap environmentMap(envMapFilePaths);

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

        // Clear previous output
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set model matrix
        const glm::mat4 model = glm::mat4(1.0f);
        
        // Draw the requested option and environment map if desired
        // Environment map must be drawn first to allow for model to overwrite it later
        if (config.currentRender == RenderOption::Combined && config.showEnvironmentMap) {
            environmentMap.render(trackball.projectionMatrix(), trackball.forward(), trackball.up());
        }
        refractionRender.draw(meshManager.getMesh(),
                              model, trackball.viewMatrix(), trackball.projectionMatrix(),
                              trackball.position(), environmentMap.getTexId());

        // Render UI
        menu.draw();

        // Present result to the screen.
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
