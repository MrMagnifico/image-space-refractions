#pragma once
#ifndef _MESH_H_
#define _MESH_H_

#include <framework/opengl_includes.h>
#include <framework/mesh.h>
#include <framework/shader.h>
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <exception>
#include <filesystem>

struct MeshLoadingException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

// Alignment directives are to comply with std140 alignment requirements (https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#Memory_layout)
struct alignas(16) GPUMaterial {
    GPUMaterial(const Material& material);

    alignas(16) glm::vec3 kd{ 1.0f };
	alignas(16) glm::vec3 ks{ 0.0f };
	float shininess{ 1.0f };
	float transparency{ 1.0f };
};

class GPUMesh {
public:
    GPUMesh(const Mesh& cpuMesh);
    // Cannot copy a GPU mesh because it would require reference counting of GPU resources.
    GPUMesh(const GPUMesh&) = delete;
    GPUMesh(GPUMesh&&);
    ~GPUMesh();

    // Generate a number of GPU meshes from a particular model file.
    // Multiple meshes may be generated if there are multiple sub-meshes in the file
    static std::vector<GPUMesh> loadMeshGPU(std::filesystem::path filePath);

    // Cannot copy a GPU mesh because it would require reference counting of GPU resources.
    GPUMesh& operator=(const GPUMesh&) = delete;
    GPUMesh& operator=(GPUMesh&&);

    bool hasTextureCoords() const;

    // Bind VAO and call glDrawElements.
    void draw(const Shader& drawingShader) const;

private:
    void moveInto(GPUMesh&&);
    void freeGpuMemory();

private:
    static constexpr GLuint INVALID = 0xFFFFFFFF;

    GLsizei m_numIndices { 0 };
    bool m_hasTextureCoords { false };
    GLuint m_ibo { INVALID };
    GLuint m_vbo { INVALID };
    GLuint m_vao { INVALID };
    GLuint m_uboMaterial { INVALID };
};


#endif // _MESH_H_
