#include "mesh_manager.h"

#include <framework/mesh.h>
#include <framework/ray.h>

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <cereal/archives/binary.hpp>
DISABLE_WARNINGS_POP()
#include <omp.h>

#include <ray_tracing/bounding_volume_hierarchy.h>
#include <utils/constants.h>
#include <utils/progressbar.hpp>


#include <fstream>
#include <iostream>


MeshManager::MeshManager(const Config& config, const std::filesystem::path& filePath)
    : m_config(config) {
    if (!std::filesystem::exists(utils::CACHE_PATH)) { std::filesystem::create_directory(utils::CACHE_PATH); }
    loadNewMesh(filePath);
}

void MeshManager::loadNewMesh(const std::filesystem::path& filePath) {
    // Construct file path for cache file
    std::filesystem::path cacheFile = filePath.filename();
    cacheFile.replace_extension("cache");
    std::filesystem::path cachePath = utils::CACHE_PATH / cacheFile;

    // Acquire mesh on the CPU from either a cache file or by doing computations on a model file
    Mesh cpuMesh;
    if (std::filesystem::exists(cachePath)) {
        std::cout << "Loading cached file " << cachePath << std::endl;
        cpuMesh = loadCached(cachePath);
    }
    else {
        std::cout << "Loading model file " << filePath << std::endl;
        cpuMesh = loadAndComputeDist(filePath);
        saveMeshCache(cpuMesh, cachePath);
    }

    // Free old mesh (if it exists) and Load new mesh onto the GPU
    m_mesh.reset(new GPUMesh(cpuMesh));
}

Mesh MeshManager::loadAndComputeDist(const std::filesystem::path& modelPath) {
    // Load mesh into CPU and construct BVH
    std::vector<Mesh> allLoadedMeshes   = loadMesh(modelPath, true);
    Mesh& mainMeshCPU                   = allLoadedMeshes[0];
    BoundingVolumeHierarchy bvh(mainMeshCPU, m_config);

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

Mesh MeshManager::loadCached(const std::filesystem::path& cachePath) {
    Mesh cpuMesh;
    std::ifstream fileStream(cachePath, std::ios::binary);
    cereal::BinaryInputArchive cacheArchive(fileStream);
    cacheArchive(cpuMesh);
    return cpuMesh;
}

void MeshManager::saveMeshCache(const Mesh& mesh, const std::filesystem::path& cachePath) {
    std::ofstream fileStream(cachePath, std::ios::binary);
    cereal::BinaryOutputArchive cacheArchive(fileStream);
    cacheArchive(mesh);
}
