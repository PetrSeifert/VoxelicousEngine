#pragma once

#include "Core.h"

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

		inline Window& GetWindow() { return *m_Window; }
		inline Device& GetDevice() { return *m_Device; }
		inline Instance& GetInstance() { return *m_Instance; }
		
		inline static App& Get() { return *s_Instance; }
	protected:
		bool OnWindowClose(const WindowCloseEvent& e);
		
		std::unique_ptr<Instance> m_Instance;
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Device> m_Device;
		std::unique_ptr<DescriptorPool> m_GlobalPool;
		std::unique_ptr<Renderer> m_Renderer;
		bool m_Running{ true };
		LayerStack m_LayerStack;

		static App* s_Instance;
	};

	// To be defined in CLIENT
	App* CreateApp();
}
