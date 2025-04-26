#pragma once

#include "Device.h"
#include "ShaderManager.h"

namespace VoxelicousEngine
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo() = default;

        std::vector<VkVertexInputBindingDescription> BindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
        VkPipelineViewportStateCreateInfo ViewportInfo;
        VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo RasterizationInfo;
        VkPipelineMultisampleStateCreateInfo MultisampleInfo;
        VkPipelineColorBlendAttachmentState ColorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
        std::vector<VkDynamicState> DynamicStateEnables;
        VkPipelineDynamicStateCreateInfo DynamicStateInfo;
        VkPipelineLayout PipelineLayout = nullptr;
        VkRenderPass RenderPass = nullptr;
        uint32_t Subpass{0};
    };

    class Pipeline
    {
    public:
        Pipeline(
            Device& device,
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline() = delete;

        void Bind(VkCommandBuffer commandBuffer) const;

        static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        
        // Static method to access the shared shader manager
        static ShaderManager& GetShaderManager() 
        { 
            static ShaderManager s_ShaderManager;
            return s_ShaderManager; 
        }

    private:
        // This replaces the old ReadFile method
        std::vector<uint32_t> LoadShader(const std::string& filePath, ShaderType type);

        void CreateGraphicsPipeline(
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& configInfo);

        void CreateShaderModule(const std::vector<uint32_t>& code, VkShaderModule* shaderModule) const;

        Device& m_Device;
        VkPipeline m_GraphicsPipeline;
        VkShaderModule m_VertShaderModule;
        VkShaderModule m_FragShaderModule;
    };
}
