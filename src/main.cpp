#include <framework/opengl_includes.h>
#include <framework/ray.h>
#include <framework/trackball.h>
#include <framework/window.h>

#include <ray_tracing/bounding_volume_hierarchy.h>
#include <render/environment_map.h>
#include <render/mesh.h>
#include <render/refraction.h>
#include <ui/menu.h>
#include <utils/config.h>
#include <utils/constants.h>
#include <utils/numerical_utils.h>
#include <utils/progressbar.hpp>

#include <omp.h>

#include <iostream>
#include <vector>


int main(int argc, char* argv[]) {
    // Init core objects
    Config config;
    Window window { "Interactive Refraction", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46 };
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

    // Load mesh into CPU and construct BVH
    std::vector<Mesh> allLoadedMeshes   = loadMesh(utils::RESOURCES_PATH / "spot.obj", true);
    Mesh& mainMeshCPU                   = allLoadedMeshes[0];
    BoundingVolumeHierarchy bvh(mainMeshCPU, config);

    // Compute d_N for every vertex in the mesh
    // We have to use an index-based loop WITH A FUCKING SIGNED INT because MSVC OpenMP support is stuck in 2006
    HitInfo dummyHit;
    progressbar progressbar(static_cast<int32_t>(mainMeshCPU.vertices.size()));
    std::cout << "Computing inner distances..." << std::endl;
    #pragma omp parallel for
    for (int32_t vertexIdx = 0; vertexIdx < mainMeshCPU.vertices.size(); vertexIdx++) {
        Vertex& vertex          = mainMeshCPU.vertices[vertexIdx];
        glm::vec3 reverseNormal = -vertex.normal;
        Ray interiorRay = {
            .origin     = vertex.position + utils::INTERIOR_RAY_OFFSET * reverseNormal,
            .direction  = reverseNormal,
            .t          = std::numeric_limits<float>::max()
        };
        bvh.intersect(interiorRay, dummyHit);
        vertex.distanceInner = interiorRay.t;

        #pragma omp critical
        progressbar.update();
    }
    std::cout << std::endl << "Finished computing inner distances!" << std::endl;

    // Load mesh into GPU
    GPUMesh mainMeshGPU(mainMeshCPU);

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
        
        // Draw the requested option and environment map if desired
        refractionRender.draw(mainMeshGPU, model, trackball.viewMatrix(), trackball.projectionMatrix(), trackball.position(), environmentMap.getTexId());
        if (config.currentRender == RenderOption::Combined && config.showEnvironmentMap) {
            environmentMap.render(trackball.viewMatrix(), trackball.projectionMatrix(), trackball.position());
        }

        // Render UI
        menu.draw();

        // Present result to the screen.
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
