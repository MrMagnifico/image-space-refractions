#pragma once
#ifndef _MESH_MANAGER_H_
#define _MESH_MANAGER_H_

#include <render/mesh.h>
#include <utils/config.h>

#include <filesystem>
#include <memory>

class MeshManager {
public:
    MeshManager(const Config& config, const std::filesystem::path& filePath);

    GPUMesh& getMesh() { return *m_mesh; }
    void loadNewMesh(const std::filesystem::path& filePath);

private:
    Mesh loadAndComputeDist(const std::filesystem::path& modelPath);
    Mesh loadCached(const std::filesystem::path& cachePath);
    void saveMeshCache(const Mesh& mesh, const std::filesystem::path& cachePath);

    const Config& m_config;
    std::unique_ptr<GPUMesh> m_mesh;
};


#endif // _MESH_MANAGER_H_
