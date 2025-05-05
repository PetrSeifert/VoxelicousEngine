#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace VoxelicousEngine 
{
    struct VoxelMaterial {
        glm::vec3 Color{1.0f, 1.0f, 1.0f};
        float EmissionStrength{0.0f};
        // Add other material properties like roughness, metalness, etc. later
    };

    struct Voxel {
        uint32_t MaterialID{0}; // Index into a material list/buffer
        bool IsSolid{false}; 
        // Add other voxel-specific data if needed
    };
} 