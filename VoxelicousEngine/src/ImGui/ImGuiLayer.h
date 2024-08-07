#pragma once

#include "Core/Layer.h"

#include "Renderer/Descriptors.h"

namespace VoxelicousEngine
{
    class ImGuiLayer final : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;

        void Begin();
        void End();

        void BlockEvents(const bool block) { m_BlockEvents = block; }
        
        void SetDarlThemeColors();

        uint32_t GetActiveWidgetID() const;

    private:
        std::unique_ptr<DescriptorPool> m_GlobalPool;
        
        bool m_BlockEvents = true;
    };
}
