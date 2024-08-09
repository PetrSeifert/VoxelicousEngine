﻿#pragma once

#include "ImGui/ImGuiLayer.h"
#include "Renderer/RenderCommandBuffer.h"

namespace VoxelicousEngine
{
    class VulkanImGuiLayer : public ImGuiLayer
    {
    public:
        VulkanImGuiLayer();
        VulkanImGuiLayer(const std::string& name);
        virtual ~VulkanImGuiLayer();

        virtual void Begin() override;
        virtual void End() override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;
    private:
        Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
    };

}