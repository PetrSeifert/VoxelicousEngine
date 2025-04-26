#include "vepch.h"
#include "SimpleRenderSystem.h"
#include "Core/Core.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <ranges>

namespace VoxelicousEngine
{
    struct SimplePushConstantData
    {
        glm::mat4 ModelMatrix{1.f};
    };

    SimpleRenderSystem::SimpleRenderSystem(Device& device, const VkRenderPass renderPass,
                                           const VkDescriptorSetLayout globalSetLayout) : m_Device(device)
    {
        CreatePipelineLayout(globalSetLayout);
        CreatePipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    }

    void SimpleRenderSystem::CreatePipelineLayout(const VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        const std::vector descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
        {
            VE_CORE_ERROR("Failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::CreatePipeline(const VkRenderPass renderPass)
    {
        VE_CORE_ASSERT(m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.RenderPass = renderPass;
        pipelineConfig.PipelineLayout = m_PipelineLayout;
        
        // Use non-SPV shader files - the ShaderManager will compile them automatically
        m_Pipeline = std::make_unique<Pipeline>(
            m_Device,
            "shaders/simple.vert",
            "shaders/simple.frag",
            pipelineConfig
        );
        
        // Check for shader changes periodically
        Pipeline::GetShaderManager().CheckForChanges();
    }

    void SimpleRenderSystem::RenderGameObjects(const FrameInfo& frameInfo) const
    {
        // Check for shader changes before rendering
        Pipeline::GetShaderManager().CheckForChanges();
        
        m_Pipeline->Bind(frameInfo.CommandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0,
            1,
            &frameInfo.GlobalDescriptorSet,
            0,
            nullptr
        );

        for (auto& val : frameInfo.GameObjects | std::views::values)
        {
            auto& obj = val;
            if (obj.Model == nullptr) continue;
            SimplePushConstantData push{};
            push.ModelMatrix = obj.Transform.Mat4();

            vkCmdPushConstants(
                frameInfo.CommandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push
            );
            obj.Model->Bind(frameInfo.CommandBuffer);
            obj.Model->Draw(frameInfo.CommandBuffer);
        }
    }
}
