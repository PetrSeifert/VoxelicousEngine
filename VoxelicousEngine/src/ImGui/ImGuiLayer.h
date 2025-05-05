#pragma once

#include "Core/Window.h"
#include "Core/Layer.h"
#include "Renderer/Buffer.h"
#include "Renderer/Descriptors.h"
#include "Core/App.h"
#include "Renderer/Renderer.h"
#include "Core/KeyboardCameraController.h"

namespace VoxelicousEngine
{
    class ImGuiLayer final : public Layer
    {
    public:
        ImGuiLayer(Renderer& renderer, Device& device, DescriptorPool& globalPool);
        ~ImGuiLayer() override;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
        void OnRender(VkCommandBuffer commandBuffer) override;
        void OnEvent(Event& event) override;

    private:
        Renderer& m_Renderer;
        Device& m_Device;
        DescriptorPool& m_GlobalPool;
        Window& m_Window = App::Get().GetWindow();

        std::chrono::steady_clock::time_point m_CurrentTime;

        std::unique_ptr<DescriptorSetLayout> m_GlobalSetLayout = DescriptorSetLayout::Builder(m_Device)
                                                                 .AddBinding(
                                                                     0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                     VK_SHADER_STAGE_ALL_GRAPHICS)
                                                                 .Build();
    };
}
