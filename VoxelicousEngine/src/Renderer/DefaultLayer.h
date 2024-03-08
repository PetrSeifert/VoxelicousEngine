#pragma once

#include "Core/Window.h"
#include "Core/Layer.h"
#include "Buffer.h"
#include "Descriptors.h"
#include "Core/App.h"
#include "Renderer.h"
#include "Camera.h"
#include "SimpleRenderSystem.h"
#include "Core/KeyboardCameraController.h"

namespace VoxelicousEngine
{
    class DefaultLayer final : public Layer
    {
    public:
        explicit DefaultLayer(Renderer& renderer, Device& device, DescriptorPool& globalPool);
        ~DefaultLayer() override;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(VkCommandBuffer commandBuffer) override;
        void OnEvent(Event& event) override;

    private:
        Renderer& m_Renderer;
        Device& m_Device;
        DescriptorPool& m_GlobalPool;
        Window& m_Window = App::Get().GetWindow();
        GameObject m_ViewerObject = GameObject::CreateGameObject();
        Camera m_Camera{};
        KeyboardCameraController m_CameraController;

        std::chrono::steady_clock::time_point m_CurrentTime;


        std::vector<std::unique_ptr<Buffer>> m_UboBuffers;
        std::vector<VkDescriptorSet> m_GlobalDescriptorSets;

        std::unique_ptr<DescriptorSetLayout> m_GlobalSetLayout = DescriptorSetLayout::Builder(m_Device)
                                                                 .AddBinding(
                                                                     0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                     VK_SHADER_STAGE_ALL_GRAPHICS)
                                                                 .Build();

        SimpleRenderSystem m_SimpleRendererSystem{
            m_Device,
            m_Renderer.GetSwapChainRenderPass(),
            m_GlobalSetLayout->GetDescriptorSetLayout()
        };

        GameObject::Map m_GameObjects;
    };
}
