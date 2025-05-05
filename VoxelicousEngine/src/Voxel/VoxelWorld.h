#pragma once

#include "SparseVoxelOctree.h"
#include "VoxelData.h"
#include <memory>
#include <vector>

namespace VoxelicousEngine {

    class VoxelWorld {
    public:
        VoxelWorld(uint32_t octreeMaxDepth, float octreeSize, const glm::vec3& octreeCenter);
        ~VoxelWorld() = default;

        VoxelWorld(const VoxelWorld&) = delete;
        VoxelWorld& operator=(const VoxelWorld&) = delete;

        // Generates some simple test geometry
        void GenerateTestData();

        // Access the octree for ray tracing
        const SparseVoxelOctree& GetOctree() const { return *m_Octree; }
        
        // Access materials (needed for shading)
        const std::vector<VoxelMaterial>& GetMaterialPalette() const { return m_MaterialPalette; }


    private:
        std::unique_ptr<SparseVoxelOctree> m_Octree;
        std::vector<VoxelMaterial> m_MaterialPalette;

        // Helper to add a material and return its ID
        uint32_t AddMaterial(const VoxelMaterial& material);
    };

} // namespace VoxelicousEngine 