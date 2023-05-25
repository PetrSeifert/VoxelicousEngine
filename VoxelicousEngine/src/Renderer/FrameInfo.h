#pragma once

#include "Camera.h"
#include "Core/GameObject.h"

#include <vulkan/vulkan.h>

namespace VoxelicousEngine
{
	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera& camera;
		VkDescriptorSet globalDescriptorSet;
		GameObject::Map& gameObjects;
	};
}