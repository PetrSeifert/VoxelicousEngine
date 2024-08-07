#pragma once

#include "Core.h"
#include "TimeStep.h"
#include "Events/Event.h"

namespace VoxelicousEngine
{
    class Layer
    {
    public:
        explicit Layer(std::string name = "Layer");
        
        virtual ~Layer() = default;

        virtual void OnAttach(){}

        virtual void OnDetach(){}

        virtual void OnUpdate(TimeStep timeStep){}
        
		virtual void OnImGuiRender() {}

        virtual void OnEvent(Event& event){}

        const std::string& GetName() const { return m_DebugName; }

    private:
        std::string m_DebugName;
    };
}
