#pragma once
#ifndef _BOUNDING_VOLUME_HIERARCHY_H_
#define _BOUNDING_VOLUME_HIERARCHY_H_

#include <framework/mesh.h>
#include <framework/ray.h>
#include <ray_tracing/common.h>
#include <utils/config.h>

#include <array>
#include <span>
#include <vector>

// Forward declaration.
struct Scene;

// A primitive represents a triangle stored inside the BVH's leaf nodes
struct Primitive {
    Vertex v0, v1, v2;  // Stored vertices, forming a triangle

    // Default equality operator (not relevant for understanding the code)
    [[nodiscard]] constexpr bool operator==(const Primitive&) const noexcept = default;

    [[nodiscard]] glm::vec3 centroid() const;

    static bool xAxisSort(const Primitive& lhs, const Primitive& rhs) { return lhs.centroid().x < rhs.centroid().x; }
    static bool yAxisSort(const Primitive& lhs, const Primitive& rhs) { return lhs.centroid().y < rhs.centroid().y; }
    static bool zAxisSort(const Primitive& lhs, const Primitive& rhs) { return lhs.centroid().z < rhs.centroid().z; }
};

// Packed BVH node; a node either has two children, or it is a leaf, in
// which case it refers to one or more primitives (triangles)
struct Node {
    static constexpr uint32_t LeafBit = 1u << 31; // A flag bit used to distinguish nodes and leaves

    AxisAlignedBox aabb; // Bounding box around the node's contained primitives

    // Node data; the first integer's most significant bit is used as a flag
    // to determine the node's purpose as a leaf or node.
    // In short, the layout is as follows:
    // - node: [[0, index of left child], [1, index of right child]]
    // - leaf: [[0, offset to primitive], [1, count of primitives]]
    std::array<uint32_t, 2> data;

    // Return if the node is a leaf node
    [[nodiscard]] inline constexpr bool isLeaf() const { return (data[0] & LeafBit) == LeafBit; };

public: // Getters
    [[nodiscard]] inline constexpr uint32_t primitiveOffset()   const { return data[0] & (~LeafBit); }
    [[nodiscard]] inline constexpr uint32_t primitiveCount()    const { return data[1]; }
    [[nodiscard]] inline constexpr uint32_t leftChild()         const { return data[0]; }
    [[nodiscard]] inline constexpr uint32_t rightChild()        const { return data[1]; }
};
static_assert(sizeof(Node) == 32);

class BoundingVolumeHierarchy {
public:
    static constexpr size_t LeafSize = 4ULL;   // Maximum nr. of primitives in a leaf

    // Constructor. Receives the scene and builds the bounding volume hierarchy.
    BoundingVolumeHierarchy(const Mesh& mesh, const Config& config);

    // Return how many levels there are in the tree that you have constructed.
    [[nodiscard]] int numLevels() const;

    // Return how many leaf nodes there are in the tree that you have constructed.
    [[nodiscard]] size_t numLeaves() const;

    // Return true if something is hit, returns false otherwise.
    // Only find hits if they are closer than t stored in the ray and the intersection
    // is on the correct side of the origin (the new t >= 0).
    bool intersect(Ray& ray, HitInfo& hitInfo) const;

    // Getters
    std::span<const Node> nodes() const             { return m_nodes; }
    std::span<Node> nodes()                         { return m_nodes; }
    std::span<const Primitive> primitives() const   { return m_primitives; }
    std::span<Primitive> primitives()               { return m_primitives; }

private:
    const Mesh& m_mesh;
    const Config& m_config;

    int m_numLevels = 0;
    uint32_t m_rootIdx;
    std::vector<Primitive> m_primitives;    // Primitives covered by leaf nodes
    std::vector<Node> m_nodes;              // Nodes comprising BVH
    std::vector<uint32_t> m_leafIndices;    // Indices of leaf nodes in m_nodes vector

    // ========== INTERSECTION METHODS ==========
    bool intersectNaive(Ray& ray, HitInfo& hitInfo) const;
    bool intersectAcceleratedRecursive(const Node& currentNode, Ray& ray, HitInfo& hitInfo) const;
    bool intersectAccelerated(Ray& ray, HitInfo& hitInfo) const;

    // ========== CREATION METHODS ==========
    /**
     * Recursively constructed a BVH rooted at the node covering the given primitives
     * 
     * @param primitives Primitives that the node should cover
     * @param currentLevel Level in the overall BVH that the node lies in
     * 
     * @return Index of the constructed node in the node vector
    */
    uint32_t constructRecursive(const std::span<Primitive>& primitives, int currentLevel);

    // ========== GENERAL UTILITIES ==========
    // Build a vector of primitives covering all triangles in the scene's mesh
    std::vector<Primitive> buildPrimitives() const;

    // Construct a bounding box spanning all given triangles
    AxisAlignedBox boundingBox(const std::span<Primitive>& triangles) const;

    /**
     * Compute the longest axis of the given AABB
     * 
     * @param box AABB to operate on
     * 
     * @return Number indicating which axis is the longest (0 => X, 1 => Y, 2 => Z)
    */
    uint32_t longestAxis(const AxisAlignedBox& box) const;
};


#endif // _BOUNDING_VOLUME_HIERARCHY_H_
