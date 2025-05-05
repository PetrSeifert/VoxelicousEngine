#include "vepch.h"
#include "SparseVoxelOctree.h"
#include "Core/Core.h"
#include "Core/Log.h"
#include <cmath> // For pow, floor, etc.
#include <algorithm> // For std::max, std::min

namespace VoxelicousEngine {

    // --- Constructor ---
    SparseVoxelOctree::SparseVoxelOctree(uint32_t maxDepth, float size, const glm::vec3& center)
        : m_MaxDepth(maxDepth),
          m_Size(size),
          m_Center(center),
          m_RootMinBounds(center - glm::vec3(size * 0.5f)), // Calculate root min bounds
          m_GridDimension(static_cast<uint32_t>(std::pow(2, maxDepth))),
          m_VoxelSize(size / m_GridDimension) 
    {
        VE_CORE_ASSERT(maxDepth > 0 && maxDepth < 16, "Max depth must be reasonable (1-15)"); // Sanity check depth
        VE_CORE_ASSERT(size > 0.0f, "Size must be positive");

        // Initialize node pool
        // Index 0 is reserved (represents null/empty)
        m_NodePool.emplace_back(); // Dummy node at index 0

        // Allocate the root node (index 1)
        m_NodePool.emplace_back(); // Root node

        // Initialize voxel palette with a default "empty" voxel at index 0
        m_VoxelPalette.emplace_back(); // Default Voxel (IsSolid=false)
        // Root node initially points to this empty voxel data
        GetNode(1).DataIndex = 0; 
    }

    // --- Node Management ---
    uint32_t SparseVoxelOctree::AllocateChildrenForNode(uint32_t parentNodeIndex) {
        // Check if children already allocated (should ideally not happen if called correctly)
        if (!GetNode(parentNodeIndex).IsLeaf()) {
            // VE_CORE_WARN("Attempting to re-allocate children for node {}", parentNodeIndex);
            return GetNode(parentNodeIndex).FirstChildIndex; // Return existing index
        }

        uint32_t firstChildIndex = static_cast<uint32_t>(m_NodePool.size());
        uint32_t parentDataIndex = GetNode(parentNodeIndex).DataIndex; // Propagate parent's data initially

        // Allocate 8 contiguous nodes
        for (int i = 0; i < 8; ++i) {
            m_NodePool.emplace_back(); 
            m_NodePool.back().DataIndex = parentDataIndex; // Inherit parent's data
        }

        // Link parent to the first child
        GetNode(parentNodeIndex).FirstChildIndex = firstChildIndex;
        // Parent node no longer stores direct data after subdivision
        GetNode(parentNodeIndex).DataIndex = 0; // Or set a flag indicating internal node

        return firstChildIndex;
    }

    // --- Palette Management ---
    uint32_t SparseVoxelOctree::FindOrAddVoxelData(const Voxel& voxelData) {
        // Simple linear search for now. Could optimize with a hash map if palette gets large.
        for (uint32_t i = 0; i < m_VoxelPalette.size(); ++i) {
            // Need a proper comparison for Voxel struct
            if (m_VoxelPalette[i].MaterialID == voxelData.MaterialID && 
                m_VoxelPalette[i].IsSolid == voxelData.IsSolid) { // Add other fields if Voxel struct grows
                return i;
            }
        }
        
        // Not found, add it
        uint32_t newIndex = static_cast<uint32_t>(m_VoxelPalette.size());
        m_VoxelPalette.push_back(voxelData);
        return newIndex;
    }

    // --- Coordinate/Index Calculations ---

    // Determines which of the 8 child octants contains the target coordinates
    int SparseVoxelOctree::GetChildIndex(const glm::ivec3& targetCoords, const glm::ivec3& nodeCenterCoords) const {
        int index = 0;
        if (targetCoords.x >= nodeCenterCoords.x) index |= 1; // Right half
        if (targetCoords.y >= nodeCenterCoords.y) index |= 2; // Top half
        if (targetCoords.z >= nodeCenterCoords.z) index |= 4; // Front half
        return index;
    }

    // Calculates the minimum integer coordinates of a child node
    glm::ivec3 SparseVoxelOctree::GetChildMinCoords(const glm::ivec3& parentMinCoords, int childIndex, uint32_t parentDepth) const {
        uint32_t childGridDim = m_GridDimension / static_cast<uint32_t>(std::pow(2, parentDepth + 1));
        glm::ivec3 offset(0);
        if (childIndex & 1) offset.x = childGridDim; // Right
        if (childIndex & 2) offset.y = childGridDim; // Top
        if (childIndex & 4) offset.z = childGridDim; // Front
        return parentMinCoords + offset;
    }

    // --- Public Voxel Manipulation ---

    void SparseVoxelOctree::SetVoxel(const glm::ivec3& coords, const Voxel& voxelData) {
        // Check if coordinates are within the octree bounds
        if (glm::any(glm::lessThan(coords, glm::ivec3(0))) || glm::any(glm::greaterThanEqual(coords, glm::ivec3(m_GridDimension)))) {
            // VE_CORE_WARN("Attempted to set voxel outside octree bounds at ({}, {}, {})", coords.x, coords.y, coords.z);
            return; // Or handle as an error
        }

        uint32_t dataIndex = FindOrAddVoxelData(voxelData);
        // Start recursion from the root node (index 1), depth 0
        SetVoxelRecursive(1, coords, voxelData, glm::ivec3(0), 0); 
    }

    // --- Recursive Voxel Helpers ---

    void SparseVoxelOctree::SetVoxelRecursive(uint32_t nodeIndex, const glm::ivec3& targetCoords, const Voxel& voxelData, const glm::ivec3& nodeMinCoords, uint32_t depth) {
        // Get reference initially, but be aware it might dangle after AllocateChildrenForNode
        OctreeNode& nodeRef = GetNode(nodeIndex); 
        uint32_t nodeGridDim = m_GridDimension / static_cast<uint32_t>(std::pow(2, depth));

        // Base case: reached max depth (leaf node representing a single voxel)
        if (depth == m_MaxDepth) {
            // Use the potentially updated reference here
            GetNode(nodeIndex).DataIndex = FindOrAddVoxelData(voxelData);
            GetNode(nodeIndex).FirstChildIndex = 0; // Ensure it's marked as leaf
            return;
        }

        uint32_t firstChildIndex = nodeRef.FirstChildIndex; // Store potential FirstChildIndex before potential invalidation
        bool wasLeaf = nodeRef.IsLeaf(); // Check if it was leaf before potential invalidation

        // If the current node is a leaf (represents a uniform block), but we need to set a voxel within it
        if (wasLeaf) { 
            // Get data index before potential invalidation
            uint32_t existingDataIndex = nodeRef.DataIndex;
            uint32_t targetDataIndex = FindOrAddVoxelData(voxelData);
            
            if (existingDataIndex != targetDataIndex) {
                // Subdivide this leaf node. This might invalidate nodeRef.
                firstChildIndex = AllocateChildrenForNode(nodeIndex); 
                // After this call, nodeRef might be invalid. Do not use it directly anymore.
                // GetNode(nodeIndex) should now return the updated parent node state.
                wasLeaf = false; // It's no longer a leaf conceptually
            } else {
                // Data is the same, no need to subdivide or recurse further.
                return;
            }
        }
        
        // If we reach here, the node is either an internal node, or was just subdivided.
        // We must use GetNode(nodeIndex) or firstChildIndex which was obtained after potential allocation.

        // Calculate which child octant the target coordinates fall into
        uint32_t childNodeSize = nodeGridDim / 2; // Integer size
        glm::ivec3 nodeCenterCoords = nodeMinCoords + glm::ivec3(childNodeSize);
        int childOctantIndex = GetChildIndex(targetCoords, nodeCenterCoords);
        
        // Calculate the index and min coords for the specific child node
        // Use firstChildIndex obtained *after* potential allocation
        uint32_t childNodeIndex = firstChildIndex + childOctantIndex;
        glm::ivec3 childMinCoords = GetChildMinCoords(nodeMinCoords, childOctantIndex, depth); 

        // Recurse into the correct child
        SetVoxelRecursive(childNodeIndex, targetCoords, voxelData, childMinCoords, depth + 1);

        // --- Optional: Pruning / Merging Check ---
        // Re-fetch the parent node reference as it might have been invalidated if AllocateChildren was called.
        // It also now reflects the state *after* the recursive call returned.
        const OctreeNode& currentParentNode = GetNode(nodeIndex);
        
        // Only attempt merge if it has children (i.e., it's not a leaf)
        if (!currentParentNode.IsLeaf()) {
            // Use the FirstChildIndex from the potentially updated parent node state
            uint32_t currentFirstChildIndex = currentParentNode.FirstChildIndex;
            uint32_t firstChildDataIndex = GetNode(currentFirstChildIndex).DataIndex;
            bool allChildrenSame = true;
            for (int i = 0; i < 8; ++i) {
                // Check child node state after recursion
                const OctreeNode& child = GetNode(currentFirstChildIndex + i); 
                if (!child.IsLeaf() || child.DataIndex != firstChildDataIndex) {
                    allChildrenSame = false;
                    break;
                }
            }

            if (allChildrenSame) {
                // Collapse. Get a mutable reference to the parent node to modify it.
                OctreeNode& mutableParentNode = GetNode(nodeIndex);
                mutableParentNode.DataIndex = firstChildDataIndex;
                mutableParentNode.FirstChildIndex = 0; // Mark as leaf again
                
                // Note: Orphaned nodes remain.
            }
        }
    }
} // namespace VoxelicousEngine 