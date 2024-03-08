#pragma once

#include "Camera.h"
#include "Core/GameObject.h"

#include <vulkan/vulkan.h>

namespace VoxelicousEngine
{
    struct FrameInfo
    {
        int FrameIndex;
        float FrameTime;
        VkCommandBuffer CommandBuffer;
        Camera& Camera;
        VkDescriptorSet GlobalDescriptorSet;
        GameObject::Map& GameObjects;
    };
}
