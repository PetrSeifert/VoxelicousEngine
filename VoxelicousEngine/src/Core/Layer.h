#pragma once

#include "vulkan/vulkan.h"
#include "Events/Event.h"

namespace VoxelicousEngine
{
    class Layer
    {
    public:
        explicit Layer(std::string name = "Layer");
        virtual ~Layer();

        virtual void OnAttach()
        {
        }

        virtual void OnDetach()
        {
        }

        virtual void OnUpdate(VkCommandBuffer commandBuffer)
        {
        }

        virtual void OnEvent(Event& event)
        {
        }

        const std::string& GetName() const { return m_DebugName; }

    private:
        std::string m_DebugName;
    };
}
