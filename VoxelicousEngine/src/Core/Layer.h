#pragma once

#include "Events/Event.h"
#include "Core/TimeStep.h"

namespace VoxelicousEngine
{
    class Layer
    {
    public:
        explicit Layer(std::string name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach(){}
        virtual void OnDetach(){}
        virtual void OnUpdate(TimeStep ts){}
		virtual void OnImGuiRender() {}
        virtual void OnEvent(Event& event){}

        const std::string& GetName() const { return m_DebugName; }

    private:
        std::string m_DebugName;
    };
}
