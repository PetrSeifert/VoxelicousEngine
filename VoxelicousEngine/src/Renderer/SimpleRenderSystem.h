#pragma once

#include "Pipeline.h"
#include "Device.h"
#include "FrameInfo.h"

namespace VoxelicousEngine
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void RenderGameObjects(const FrameInfo& frameInfo) const;

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        Device& m_Device;

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout m_PipelineLayout;
    };
}
