#include <render/environment_map.h>

#include <utils/constants.h>

#include <stb/stb_image.h>

#include <array>


EnvironmentMap::EnvironmentMap(const EnvMapFilePaths& texPaths) {
    createCubemapTex(texPaths);
    createCubeBuffers();
    compileRenderShader();
}

EnvironmentMap::~EnvironmentMap() {
    glDeleteTextures(1, &m_cubemapTex);
    glDeleteVertexArrays(1, &m_cubeVAO);
    glDeleteBuffers(1, &m_cubeVBO);
}

void EnvironmentMap::createCubemapTex(const EnvMapFilePaths& texPaths) {
    // Yes, we use the bindless API to push texture data because I can't be bothered
    // to concatenate the data of separate images
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_cubemapTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTex);
    std::array<std::filesystem::path, 6> pathsArr = { texPaths.right,   texPaths.left,
                                                      texPaths.top,     texPaths.bottom,
                                                      texPaths.front,   texPaths.back };
    for (uint8_t faceIdx = 0U; faceIdx < 6UL; faceIdx++) {
        int32_t width, height, channels;
        const std::filesystem::path& faceTexPath    = pathsArr[faceIdx];
        uint8_t* data                               = stbi_load(faceTexPath.string().c_str(), &width, &height, &channels, 3);
        if (!data) {
            stbi_image_free(data);
            throw std::runtime_error(std::format("Could not load cubemap image file: {}", faceTexPath.string()));
        }
        if (channels != 3) {
            stbi_image_free(data);
            throw std::runtime_error(std::format("Number of channels for cubemap file {} was not expected: {}", faceTexPath.string(), channels));
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    glTextureParameteri(m_cubemapTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_cubemapTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_cubemapTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_cubemapTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_cubemapTex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void EnvironmentMap::createCubeBuffers() {
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // Create VAO and VBO to house vertex info
    glCreateBuffers(1, &m_cubeVBO);
    glNamedBufferStorage(m_cubeVBO, static_cast<GLsizeiptr>(sizeof(skyboxVertices)), &skyboxVertices, 0);
    glCreateVertexArrays(1, &m_cubeVAO);
    glVertexArrayVertexBuffer(m_cubeVAO, 0, m_cubeVBO, 0, 3 * sizeof(float)); // We bind the vertex buffer to slot 0 of the VAO and tell the VBO how large each vertex is (3 floating point values)
    glEnableVertexArrayAttrib(m_cubeVAO, 0);
    glVertexArrayAttribFormat(m_cubeVAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // We tell OpenGL what each vertex looks like and how they are mapped to the shader (location = ...).
    glVertexArrayAttribBinding(m_cubeVAO, 0, 0); // For each of the vertex attributes we tell OpenGL to get them from VBO at slot 0
}

void EnvironmentMap::compileRenderShader() {
    m_renderCubeMap = ShaderBuilder().addStage(GL_VERTEX_SHADER,    utils::SHADERS_PATH / "environment_map.vert")
                                     .addStage(GL_FRAGMENT_SHADER,  utils::SHADERS_PATH / "environment_map.frag").build();
}
