#pragma once

#include "VoxelData.h"
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>
#include <limits>
#include <glm/glm.hpp>

namespace VoxelicousEngine {

    // Basic Ray definition (Consider moving to a dedicated Math header later)
    struct Ray {
        glm::vec3 Origin;
        glm::vec3 Direction;
        glm::vec3 InvDirection; // Precomputed 1.0f / Direction

        Ray(const glm::vec3& origin, const glm::vec3& direction)
            : Origin(origin), Direction(glm::normalize(direction)) // Ensure direction is normalized
        {
            // Handle division by zero for axis-aligned rays
            InvDirection.x = (Direction.x == 0.0f) ? std::numeric_limits<float>::infinity() : 1.0f / Direction.x;
            InvDirection.y = (Direction.y == 0.0f) ? std::numeric_limits<float>::infinity() : 1.0f / Direction.y;
            InvDirection.z = (Direction.z == 0.0f) ? std::numeric_limits<float>::infinity() : 1.0f / Direction.z;
        }
    };

    // --- Octree Node ---
    // Represents a node in the octree. Uses indices into a node pool.
    struct OctreeNode {
        // Index of the first child node in the pool. Children are stored contiguously (8 nodes).
        // 0 represents a null pointer (no children).
        uint32_t FirstChildIndex = 0; 

        // Index into a material/voxel data palette if this is a leaf node representing a uniform block.
        // Or potentially flags/data directly if small enough.
        uint32_t DataIndex = 0; 

        // --- Methods ---
        bool IsLeaf() const { return FirstChildIndex == 0; }
        // Add more flags/methods as needed (e.g., IsUniform, IsEmpty)
    };

    // --- Sparse Voxel Octree ---
    class SparseVoxelOctree {
    public:
        // Represents the result of a ray intersection test
        struct IntersectionResult {
            bool Hit = false;
            float Distance = std::numeric_limits<float>::max();
            glm::vec3 Normal = glm::vec3(0.0f);
            const Voxel* pHitVoxel = nullptr; // Pointer to the voxel data hit (if applicable)
            glm::ivec3 VoxelCoords = glm::ivec3(0); // Integer coordinates of the hit voxel cell
            const OctreeNode* HitNode = nullptr; // Node that was hit (leaf)
            glm::vec3 HitPoint = glm::vec3(0.0f); // World space hit point
        };


        // Constructor: Defines the octree's bounds and maximum depth.
        // size: The length of one side of the cubic root node.
        // center: The world-space center of the root node.
        SparseVoxelOctree(uint32_t maxDepth, float size, const glm::vec3& center = glm::vec3(0.0f));
        ~SparseVoxelOctree() = default; // Default destructor likely sufficient for now

        SparseVoxelOctree(const SparseVoxelOctree&) = delete;
        SparseVoxelOctree& operator=(const SparseVoxelOctree&) = delete;
        SparseVoxelOctree(SparseVoxelOctree&&) = default;
        SparseVoxelOctree& operator=(SparseVoxelOctree&&) = default;

        // --- Voxel Manipulation ---
        // Sets the voxel data at the specified integer coordinates.
        // Creates nodes along the path if they don't exist.
        void SetVoxel(const glm::ivec3& coords, const Voxel& voxelData);
        
        // --- Accessors ---
        uint32_t GetMaxDepth() const { return m_MaxDepth; }
        float GetSize() const { return m_Size; }
        const glm::vec3& GetCenter() const { return m_Center; }
        float GetVoxelSize() const { return m_VoxelSize; }
        const std::vector<OctreeNode>& GetNodePool() const { return m_NodePool; } // For potential GPU transfer
        uint32_t GetGridDimension() const { return m_GridDimension; } // Added getter
        const std::vector<Voxel>& GetVoxelPalette() const { return m_VoxelPalette; } // Added getter


    private:
        // --- Node Management ---
        uint32_t AllocateChildrenForNode(uint32_t parentNodeIndex); // Allocates 8 child nodes
        OctreeNode& GetNode(uint32_t index) { return m_NodePool[index]; }
        const OctreeNode& GetNode(uint32_t index) const { return m_NodePool[index]; }
        
        // --- Palette Management ---
        // Finds or adds a voxel definition to the palette, returning its index.
        uint32_t FindOrAddVoxelData(const Voxel& voxelData); 

        // --- Recursive Helpers ---
        void SetVoxelRecursive(uint32_t nodeIndex, const glm::ivec3& targetCoords, const Voxel& voxelData, const glm::ivec3& nodeMinCoords, uint32_t depth);

        // --- Coordinate/Index Calculations ---
        int GetChildIndex(const glm::ivec3& targetCoords, const glm::ivec3& nodeCenterCoords) const;
        glm::ivec3 GetChildMinCoords(const glm::ivec3& parentMinCoords, int childIndex, uint32_t parentDepth) const;


        // --- Member Variables ---
        uint32_t m_MaxDepth;      // Maximum depth of the tree
        float m_Size;             // World space size of the root node cube
        glm::vec3 m_Center;       // World space center of the root node
        glm::vec3 m_RootMinBounds;// Min corner of the root node's AABB
        float m_VoxelSize;        // Size of a single voxel cell at max depth
        uint32_t m_GridDimension; // Number of voxels along one dimension at max depth (2^maxDepth)

        // Node storage pool. Index 0 is unused, index 1 is the root.
        std::vector<OctreeNode> m_NodePool; 

        // Palette for storing unique Voxel data definitions. Nodes store indices into this.
        std::vector<Voxel> m_VoxelPalette; 
        // We might also need a separate Material Palette later if Voxel just stores MaterialID
        // std::vector<VoxelMaterial> m_MaterialPalette;
    };

} // namespace VoxelicousEngine 