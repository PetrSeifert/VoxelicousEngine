#pragma once

#include "Core.h"
#include "TimeStep.h"
#include "Timer.h"
#include "Window.h"
#include "LayerStack.h"
#include "RenderThread.h"

#include "AppSettings.h"
#include "Events/AppEvent.h"

#include "ImGui/ImGuiLayer.h"

#include <deque>


namespace VoxelicousEngine
{
    struct AppSpecification
    {
        std::string Name = "Voxelicous Engine";
        uint32_t WindowWidth = 1280;
        uint32_t WindowHeight = 720;
        bool WindowDecorated = false;
        bool Fullscreen = false;
        bool VSync = true;
        std::string WorkingDirectory;
        bool StartMaximized = true;
        bool Resizable = true;
        bool EnableImGui = true;
		RendererConfig RenderConfig;
		ThreadingPolicy CoreThreadingPolicy = ThreadingPolicy::MultiThreaded;
        std::filesystem::path IconPath;
    };
    
    class App
    {
        using EventCallbackFn = std::function<void(Event&)>;
    public:
        App(AppSpecification specification);
        virtual ~App();

        void Run();
        void Close();
        
        virtual void OnInit() {}
        virtual void OnShutdown();
        virtual void OnUpdate(TimeStep ts) {}
        
        virtual void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* layer);
        void RenderImGui();

        void AddEventCallback(const EventCallbackFn& eventCallback) { m_EventCallbacks.push_back(eventCallback); }

        void SetShowStats(bool show) { m_ShowStats = show; }
        
        template<typename Func>
        void QueueEvent(Func&& func)
        {
            std::scoped_lock lock(m_EventQueueMutex);
            m_EventQueue.emplace_back(true, func);
        }

        // Creates & Dispatches an event either immediately, or adds it to an event queue which will be processed after the next call
        // to SyncEvents().
        // Waiting until after next sync gives the application some control over _when_ the events will be processed.
        // An example of where this is useful:
        // Suppose an asset thread is loading assets and dispatching "AssetReloaded" events.
        // We do not want those events to be processed until the asset thread has synced its assets back to the main thread.
        template<typename TEvent, bool DispatchImmediately = false, typename... TEventArgs>
        void DispatchEvent(TEventArgs&&... args)
        {
            std::shared_ptr<TEvent> event = std::make_shared<TEvent>(std::forward<TEventArgs>(args)...);
            if constexpr (DispatchImmediately)
            {
                OnEvent(*event);
            }
            else
            {
                std::scoped_lock lock(m_EventQueueMutex);
                m_EventQueue.emplace_back(false, [event]{ App::Get().OnEvent(*event); });
            }
        }

        // Mark all waiting events as sync'd.
        // Thus allowing them to be processed on next call to ProcessEvents()
        void SyncEvents();

        Window& GetWindow() { return *m_Window; }

        static App& Get() { return *s_Instance; }

		TimeStep GetTimestep() const { return m_TimeStep; }
		TimeStep GetFrameTime() const { return m_Frametime; }
		float GetTime() const; // TODO: This should be in "Platform"

		static std::thread::id GetMainThreadID();
		static bool IsMainThread();

		static const char* GetConfigurationName();
		static const char* GetPlatformName();

		const AppSpecification& GetSpecification() const { return m_Specification; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		RenderThread& GetRenderThread() { return m_RenderThread; }
		uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
		
		AppSettings& GetSettings() { return m_AppSettings; }
		const AppSettings& GetSettings() const { return m_AppSettings; }
		
		static bool IsRuntime() { return s_IsRuntime; }
	private:
		void ProcessEvents();

		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		AppSpecification m_Specification;
		bool m_Running = true, m_Minimized = false;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		TimeStep m_Frametime;
		TimeStep m_TimeStep;
		bool m_ShowStats = true;

		RenderThread m_RenderThread;

		std::mutex m_EventQueueMutex;
		std::deque<std::pair<bool, std::function<void()>>> m_EventQueue;
		std::vector<EventCallbackFn> m_EventCallbacks;

		float m_LastFrameTime = 0.0f;
		uint32_t m_CurrentFrameIndex = 0;

		AppSettings m_AppSettings;

		static App* s_Instance;

		friend class Renderer;
	protected:
		inline static bool s_IsRuntime = false;
    };

	// Implemented by CLIENT
	App* CreateApplication(int argc, char** argv);
}
