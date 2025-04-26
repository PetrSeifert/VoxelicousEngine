#include "vepch.h"
#include "Pipeline.h"

#include "Core/Core.h"
#include "Core/Model.h"

#include <fstream>

#ifndef ENGINE_DIR
#define ENGINE_DIR ""
#endif

namespace VoxelicousEngine
{
    Pipeline::Pipeline(
        Device& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo)
        : m_Device{device}
    {
        CreateGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    Pipeline::~Pipeline()
    {
        vkDestroyShaderModule(m_Device.GetDevice(), m_VertShaderModule, nullptr);
        vkDestroyShaderModule(m_Device.GetDevice(), m_FragShaderModule, nullptr);
        vkDestroyPipeline(m_Device.GetDevice(), m_GraphicsPipeline, nullptr);
    }

    std::vector<uint32_t> Pipeline::LoadShader(const std::string& filePath, ShaderType type)
    {
        // Use the shader manager to load and potentially compile the shader
        return GetShaderManager().LoadShader(filePath, type);
    }

    void Pipeline::CreateGraphicsPipeline(
        const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo)
    {
        VE_CORE_ASSERT(configInfo.PipelineLayout != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no pipelineLayout provided in configInfo!");
        VE_CORE_ASSERT(configInfo.RenderPass != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline: no renderPass provided in configInfo!");

        // Load and potentially compile the shaders
        const auto vertCode = LoadShader(vertFilepath, ShaderType::Vertex);
        const auto fragCode = LoadShader(fragFilepath, ShaderType::Fragment);

        if (vertCode.empty() || fragCode.empty())
        {
            VE_CORE_ERROR("Failed to load/compile shaders for pipeline");
            return;
        }

        CreateShaderModule(vertCode, &m_VertShaderModule);
        CreateShaderModule(fragCode, &m_FragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = m_VertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = m_FragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto& bindingDescriptions = configInfo.BindingDescriptions;
        auto& attributeDescriptions = configInfo.AttributeDescriptions;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.InputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.ViewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.RasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.MultisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.ColorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.DepthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.DynamicStateInfo;

        pipelineInfo.layout = configInfo.PipelineLayout;
        pipelineInfo.renderPass = configInfo.RenderPass;
        pipelineInfo.subpass = configInfo.Subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(
            m_Device.GetDevice(),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &m_GraphicsPipeline) != VK_SUCCESS)
        {
            VE_CORE_ERROR("Failed to create graphics pipeline!");
        }
    }

    void Pipeline::CreateShaderModule(const std::vector<uint32_t>& code, VkShaderModule* shaderModule) const
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        if (vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            VE_CORE_ERROR("Failed to create shader module!");
        }
    }

    void Pipeline::Bind(const VkCommandBuffer commandBuffer) const
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
    }

    void Pipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
    {
        configInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.ViewportInfo.viewportCount = 1;
        configInfo.ViewportInfo.pViewports = nullptr;
        configInfo.ViewportInfo.scissorCount = 1;
        configInfo.ViewportInfo.pScissors = nullptr;

        configInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.RasterizationInfo.lineWidth = 1.0f;
        configInfo.RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        configInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.RasterizationInfo.depthBiasConstantFactor = 0.0f;
        configInfo.RasterizationInfo.depthBiasClamp = 0.0f;
        configInfo.RasterizationInfo.depthBiasSlopeFactor = 0.0f;

        configInfo.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.MultisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.MultisampleInfo.minSampleShading = 1.0f; // Optional
        configInfo.MultisampleInfo.pSampleMask = nullptr; // Optional
        configInfo.MultisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        configInfo.MultisampleInfo.alphaToOneEnable = VK_FALSE; // Optional

        configInfo.ColorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        configInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        configInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        configInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        configInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        configInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        configInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.ColorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        configInfo.ColorBlendInfo.attachmentCount = 1;
        configInfo.ColorBlendInfo.pAttachments = &configInfo.ColorBlendAttachment;
        configInfo.ColorBlendInfo.blendConstants[0] = 0.0f; // Optional
        configInfo.ColorBlendInfo.blendConstants[1] = 0.0f; // Optional
        configInfo.ColorBlendInfo.blendConstants[2] = 0.0f; // Optional
        configInfo.ColorBlendInfo.blendConstants[3] = 0.0f; // Optional

        configInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.DepthStencilInfo.minDepthBounds = 0.0f; // Optional
        configInfo.DepthStencilInfo.maxDepthBounds = 1.0f; // Optional
        configInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.DepthStencilInfo.front = {}; // Optional
        configInfo.DepthStencilInfo.back = {}; // Optional

        configInfo.DynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.DynamicStateInfo.pDynamicStates = configInfo.DynamicStateEnables.data();
        configInfo.DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.DynamicStateEnables.size());
        configInfo.DynamicStateInfo.flags = 0;

        configInfo.BindingDescriptions = Model::Vertex::GetBindingDescriptions();
        configInfo.AttributeDescriptions = Model::Vertex::GetAttributeDescriptions();
    }
}
