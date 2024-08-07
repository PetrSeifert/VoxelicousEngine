#pragma once

#include "Window.h"
#include "Renderer/Instance.h"
#include "Renderer/Renderer.h"
#include "Renderer/Device.h"
#include "Renderer/Descriptors.h"
#include "LayerStack.h"
#include "Events/Event.h"
#include "Events/AppEvent.h"

namespace VoxelicousEngine
{
    class App
    {
    public:
        App();
        virtual ~App();

        void Run();

        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        Window& GetWindow() const { return *m_Window; }
        Device& GetDevice() const { return *m_Device; }
        Instance& GetInstance() const { return *m_Instance; }

        static App& Get() { return *s_Instance; }
        
        TimeStep GetTimeStep() const { return m_TimeStep; }
        TimeStep GetFrameTime() const { return m_FrameTime; }
		float GetTime() const; // TODO: This should be in "Platform"

    protected:
        bool OnWindowClose(const WindowCloseEvent& e);

        std::unique_ptr<Instance> m_Instance;
        std::unique_ptr<Device> m_Device;
        std::unique_ptr<DescriptorPool> m_GlobalPool;
        std::unique_ptr<Renderer> m_Renderer;

    private:
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
		bool m_Minimized = false;
        LayerStack m_LayerStack;
        TimeStep m_FrameTime;
        TimeStep m_TimeStep;
        
        float m_LastFrameTime = 0.0f;
        uint32_t m_CurrentFrameIndex = 0;

        static App* s_Instance;
    };

    // To be defined in CLIENT
    App* CreateApp();
}
