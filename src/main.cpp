#include <framework/opengl_includes.h>
#include <framework/ray.h>
#include <framework/trackball.h>
#include <framework/window.h>

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <cereal/archives/binary.hpp>
DISABLE_WARNINGS_POP()

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

#include <fstream>
#include <iostream>
#include <string>
#include <vector>


Mesh computeDist(const Config& config, const std::filesystem::path& modelFile) {
    // Load mesh into CPU and construct BVH
    std::vector<Mesh> allLoadedMeshes   = loadMesh(utils::RESOURCES_PATH / modelFile, true);
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

    return mainMeshCPU;
}

int main(int argc, char* argv[]) {
    // Init core objects
    Config config;
    Window window { "Interactive Refraction", glm::ivec2(utils::WIDTH, utils::HEIGHT), OpenGLVersion::GL46 };
    Trackball trackball { &window, glm::radians(50.0f) };
    Menu menu(config);
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

    // File paths for model and its cache
    std::filesystem::path modelFile("dragon.obj");
    std::filesystem::path cacheFile = modelFile;
    cacheFile.replace_extension("cache");
    if (!std::filesystem::exists(utils::CACHE_PATH)) { std::filesystem::create_directory(utils::CACHE_PATH); }
    std::filesystem::path cachePath = utils::CACHE_PATH / cacheFile;

    // Load cached version if it exists, compute inner distance and save to archive otherwise
    Mesh cpuMesh;
    if (std::filesystem::exists(cachePath)) {
        std::cout << "Cached mesh found!" << std::endl;
        std::ifstream fileStream(cachePath, std::ios::binary);
        cereal::BinaryInputArchive cacheArchive(fileStream);
        cacheArchive(cpuMesh);
    } else {
        std::cout << "No cached mesh!" << std::endl;
        cpuMesh = computeDist(config, modelFile);
        std::ofstream fileStream(cachePath, std::ios::binary);
        cereal::BinaryOutputArchive cacheArchive(fileStream);
        cacheArchive(cpuMesh);
    }

    // Load mesh into GPU
    GPUMesh mainMeshGPU(cpuMesh);

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
        if (config.currentRender == RenderOption::Combined && config.showEnvironmentMap) {
            environmentMap.render(trackball.projectionMatrix(), trackball.forward(), trackball.up());
        }
        refractionRender.draw(mainMeshGPU, model, trackball.viewMatrix(), trackball.projectionMatrix(), trackball.position(), environmentMap.getTexId());

        // Render UI
        menu.draw();

        // Present result to the screen.
        window.swapBuffers();
    }

    return EXIT_SUCCESS;
}
