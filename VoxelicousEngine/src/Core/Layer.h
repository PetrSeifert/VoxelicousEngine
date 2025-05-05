#pragma once

#include "Core/Core.h"
#include "Events/Event.h"
#include <vulkan/vulkan.h> // Include Vulkan for VkCommandBuffer

namespace VoxelicousEngine
{
    class Layer
    {
    public:
        explicit Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach()
        {
        }

        virtual void OnDetach()
        {
        }

        virtual void OnUpdate()
        {
        }

        virtual void OnEvent(Event& event)
        {
        }

        virtual void UpdateGpuResources(VkCommandBuffer commandBuffer)
        {
        }

        virtual void OnRender(VkCommandBuffer commandBuffer)
        {
        }

        const std::string& GetName() const { return m_DebugName; }

    private:
        std::string m_DebugName;
    };
}
