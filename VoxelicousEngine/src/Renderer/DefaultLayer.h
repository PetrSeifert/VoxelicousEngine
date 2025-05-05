#pragma once

#include "Core/Window.h"
#include "Core/Layer.h"
#include "Core/App.h"
#include "Renderer.h"
#include "Camera.h"
#include "Core/KeyboardCameraController.h"
#include "Voxel/VoxelWorld.h"
#include "Renderer/VoxelRaytraceSystem.h"
#include "Pipeline.h"

namespace VoxelicousEngine
{
    class DefaultLayer final : public Layer
    {
    public:
        explicit DefaultLayer(Renderer& renderer, Device& device);
        ~DefaultLayer() override;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
        void OnEvent(Event& event) override;
        void UpdateGpuResources(VkCommandBuffer commandBuffer) override;
        void OnRender(VkCommandBuffer commandBuffer) override;

    private:
        Renderer& m_Renderer;
        Device& m_Device;
        Window& m_Window = App::Get().GetWindow();
        GameObject m_ViewerObject = GameObject::CreateGameObject();
        Camera m_Camera{};
        KeyboardCameraController m_CameraController;

        std::chrono::steady_clock::time_point m_CurrentTime;

        std::unique_ptr<VoxelWorld> m_VoxelWorld;
        std::unique_ptr<VoxelRaytraceSystem> m_VoxelRaytraceSystem;
        
        std::unique_ptr<Pipeline> m_FullscreenPipeline;
        VkPipelineLayout m_FullscreenPipelineLayout = VK_NULL_HANDLE;
    };
}
