#include "vepch.h"
#include "VoxelWorld.h"
#include "Core/Log.h"

namespace VoxelicousEngine {

    VoxelWorld::VoxelWorld(uint32_t octreeMaxDepth, float octreeSize, const glm::vec3& octreeCenter)
    {
        m_Octree = std::make_unique<SparseVoxelOctree>(octreeMaxDepth, octreeSize, octreeCenter);
        
        // Add a default "air" material at index 0 (although SVO handles empty space)
        AddMaterial({}); // Default material
    }

    uint32_t VoxelWorld::AddMaterial(const VoxelMaterial& material) {
        uint32_t id = static_cast<uint32_t>(m_MaterialPalette.size());
        m_MaterialPalette.push_back(material);
        return id;
    }

    void VoxelWorld::GenerateTestData() {
        VE_CORE_INFO("Generating voxel test data...");

        if (!m_Octree) {
            VE_CORE_ERROR("Cannot generate test data, Octree is null!");
            return;
        }

        // Example: Create some materials
        uint32_t matRed = AddMaterial({ {1.0f, 0.1f, 0.1f}, 0.0f });
        uint32_t matGreen = AddMaterial({ {0.1f, 1.0f, 0.1f}, 0.0f });
        uint32_t matBlue = AddMaterial({ {0.1f, 0.1f, 1.0f}, 0.0f });
        uint32_t matYellowEmit = AddMaterial({ {1.0f, 1.0f, 0.0f}, 5.0f }); // Emissive
        uint32_t matGray = AddMaterial({ {0.5f, 0.5f, 0.5f}, 0.0f });

        // Get octree properties
        uint32_t dim = m_Octree->GetGridDimension();
        uint32_t halfDim = dim / 2;

        // Example: Create a ground plane
        Voxel groundVoxel;
        groundVoxel.IsSolid = true;
        groundVoxel.MaterialID = matBlue;
        for (uint32_t x = 0; x < dim; ++x) {
            for (uint32_t z = 0; z < dim; ++z) {
                m_Octree->SetVoxel({x, 0, z}, groundVoxel);
            }
        }

        // Example: Place a few blocks
        Voxel blockVoxel;
        blockVoxel.IsSolid = true;

        blockVoxel.MaterialID = matRed;
        m_Octree->SetVoxel({halfDim, 1, halfDim}, blockVoxel);

        blockVoxel.MaterialID = matGreen;
        m_Octree->SetVoxel({halfDim + 2, 1, halfDim}, blockVoxel);
        m_Octree->SetVoxel({halfDim + 2, 2, halfDim}, blockVoxel);

        blockVoxel.MaterialID = matBlue;
        m_Octree->SetVoxel({halfDim, 1, halfDim + 2}, blockVoxel);

        blockVoxel.MaterialID = matYellowEmit;
        m_Octree->SetVoxel({halfDim + 1, 3, halfDim + 1}, blockVoxel);
        
        VE_CORE_INFO("Finished generating voxel test data. Octree node pool size: {}", m_Octree->GetNodePool().size());
    }

} // namespace VoxelicousEngine 