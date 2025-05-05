#include "vepch.h"
#include "DefaultLayer.h"
#include "Core/Log.h"

namespace VoxelicousEngine
{
    DefaultLayer::DefaultLayer(Renderer& renderer, Device& device) : Layer("DefaultLayer"),
        m_Renderer(renderer), m_Device(device)
    {
    }

    DefaultLayer::~DefaultLayer()
    {
        if (m_FullscreenPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_Device.GetDevice(), m_FullscreenPipelineLayout, nullptr);
        }
    }

    void DefaultLayer::OnAttach()
    {
        VE_CORE_INFO("Attaching DefaultLayer");
        uint32_t octreeDepth = 8;
        float octreeSize = 64.0f;
        m_VoxelWorld = std::make_unique<VoxelWorld>(octreeDepth, octreeSize, glm::vec3(octreeSize * 0.5f));
        m_VoxelWorld->GenerateTestData();
        
        // Initialize camera projection with correct aspect ratio
        uint32_t width = m_Window.GetWidth();
        uint32_t height = m_Window.GetHeight();
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        m_Camera.SetPerspectiveProjection(glm::radians(60.f), aspect, 0.1f, 100.f);
        
        m_VoxelRaytraceSystem = std::make_unique<VoxelRaytraceSystem>(m_Device, m_Window, *m_VoxelWorld, m_Camera);

        VkDescriptorSetLayout fullscreenSetLayout = m_VoxelRaytraceSystem->GetOutputTextureDescriptorSetLayout();
        VE_CORE_ASSERT(fullscreenSetLayout != VK_NULL_HANDLE, "Failed to get descriptor set layout from VoxelRaytraceSystem");
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &fullscreenSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_FullscreenPipelineLayout) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create fullscreen pipeline layout!");
            throw std::runtime_error("failed to create fullscreen pipeline layout!");
        }

        PipelineConfigInfo pipelineConfig{};
        Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.RenderPass = m_Renderer.GetSwapChainRenderPass(); 
        pipelineConfig.PipelineLayout = m_FullscreenPipelineLayout;
        pipelineConfig.BindingDescriptions.clear();
        pipelineConfig.AttributeDescriptions.clear();
        pipelineConfig.DepthStencilInfo.depthTestEnable = VK_FALSE;
        pipelineConfig.DepthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.ColorBlendAttachment.blendEnable = VK_FALSE;

        m_FullscreenPipeline = std::make_unique<Pipeline>(
            m_Device,
            "shaders/fullscreen.vert",
            "shaders/fullscreen.frag",
            pipelineConfig
        );
        VE_CORE_INFO("Fullscreen pipeline created.");

        m_CurrentTime = std::chrono::steady_clock::now();
    }

    void DefaultLayer::OnDetach()
    {
        VE_CORE_INFO("Detaching DefaultLayer");
    }

    void DefaultLayer::UpdateGpuResources(VkCommandBuffer commandBuffer) {
        if (m_VoxelRaytraceSystem) {
            // Get frame index from the renderer
            int frameIndex = m_Renderer.GetFrameIndex(); 
            // Dispatch the compute shader
            m_VoxelRaytraceSystem->DispatchCompute(commandBuffer, frameIndex);
        }
    }

    void DefaultLayer::OnUpdate()
    {
        auto newTime = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration<float>(newTime - m_CurrentTime).count();
        m_CurrentTime = newTime;

        m_CameraController.MoveInPlaneXZ(m_Window.GetGLFW_Window(), frameTime, m_ViewerObject);
        m_Camera.SetViewYXZ(m_ViewerObject.Transform.Translation, m_ViewerObject.Transform.Rotation);
    }

    void DefaultLayer::OnRender(VkCommandBuffer commandBuffer) {
         if (!m_FullscreenPipeline || !m_VoxelRaytraceSystem) {
             VE_CORE_WARN("Attempting to render DefaultLayer before pipeline or raytrace system is ready.");
             return;
         }
         
         m_FullscreenPipeline->Bind(commandBuffer);

         VkDescriptorSet descSet = m_VoxelRaytraceSystem->GetOutputTextureDescriptorSet();
         // Ensure descriptor set is valid (might be null if system failed init)
         if (descSet == VK_NULL_HANDLE) { 
              VE_CORE_ERROR("VoxelRaytraceSystem descriptor set is null during render!");
              return;
         }

         vkCmdBindDescriptorSets(
             commandBuffer,
             VK_PIPELINE_BIND_POINT_GRAPHICS,
             m_FullscreenPipelineLayout,
             0, // first set
             1, // set count
             &descSet,
             0, // dynamic offset count
             nullptr); // dynamic offsets

         // Draw fullscreen triangle (3 vertices)
         vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

    void DefaultLayer::OnEvent(Event& event)
    {
        if (event.GetEventType() == EventType::WindowResize) {
            auto& resizeEvent = static_cast<WindowResizeEvent&>(event);
            if (m_VoxelRaytraceSystem) {
                m_VoxelRaytraceSystem->OnResize(resizeEvent.GetWidth(), resizeEvent.GetHeight());
            }
            if (resizeEvent.GetHeight() > 0) {
                 const float aspect = static_cast<float>(resizeEvent.GetWidth()) / static_cast<float>(resizeEvent.GetHeight());
                 m_Camera.SetPerspectiveProjection(glm::radians(60.f), aspect, .1f, 100.f);
            }
        }
    }
}
