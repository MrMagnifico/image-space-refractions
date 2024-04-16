#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/glm_compat.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include "image.h"

#include <filesystem>
#include <limits>
#include <optional>
#include <span>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord; 										// Texture coordinate
	float distanceInner { std::numeric_limits<float>::max() };	// Distance to the nearest point on the interior of the mesh along normal (d_N in the paper)

	[[nodiscard]] constexpr bool operator==(const Vertex&) const noexcept = default;

	template<class Archive>
	void serialize(Archive& ar) { ar(position, normal, texCoord, distanceInner); }
};

struct Material {
	glm::vec3 kd{ 1.0f }; // Diffuse color
	glm::vec3 ks{ 0.0f };
	float shininess{ 1.0f };
	float transparency{ 1.0f };

	// Optional texture that replaces kd; use as follows:
	// 
	// if (material.kdTexture) {
	//   material.kdTexture->getTexel(...);
	// }
	std::shared_ptr<Image> kdTexture;

	template<class Archive>
	void serialize(Archive& ar) { ar(kd, ks, shininess, transparency); }
};

struct Mesh {
	// Vertices contain the vertex positions and normals of the mesh.
	std::vector<Vertex> vertices;
	// A triangle contains a triplet of values corresponding to the indices of the 3 vertices in the vertices array.
	std::vector<glm::uvec3> triangles;

	Material material;

	template<class Archive>
	void serialize(Archive& ar) { ar(CEREAL_NVP(vertices), CEREAL_NVP(triangles), material); }
};

[[nodiscard]] std::vector<Mesh> loadMesh(const std::filesystem::path& file, bool normalize = false);
[[nodiscard]] Mesh mergeMeshes(std::span<const Mesh> meshes);
void meshFlipX(Mesh& mesh);
void meshFlipY(Mesh& mesh);
void meshFlipZ(Mesh& mesh);