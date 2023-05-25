#include "vepch.h"
#include "App.h"

#include "Log.h"
#include "Renderer/SwapChain.h"

#include <GLFW/glfw3.h>

namespace VoxelicousEngine 
{

#define BIND_EVENT_FN(x) std::bind(&App::x, this, std::placeholders::_1)

	static void GLFWErrorCallback(int error, const char* description)
	{
		VE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	App* App::s_Instance = nullptr;

	App::App()
	{
		VE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		int success = glfwInit();
		VE_CORE_ASSERT(success, "Could not intialize GLFW!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwSetErrorCallback(GLFWErrorCallback);

		m_Instance = std::make_unique<Instance>();

		m_Window = std::unique_ptr<Window>(Window::Create(*m_Instance));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_Device = std::make_unique<Device>(m_Instance->Get(), m_Window->GetVkSurfaceKHR());
		m_GlobalPool = DescriptorPool::Builder(*m_Device)
			.SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
			.SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.Build();

		m_Renderer = std::make_unique<Renderer>(*m_Window, *m_Device);
	}

	App::~App()
	{
		s_Instance = nullptr;
	}

	void App::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void App::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void App::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}


	void App::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApp))
		{
			VE_TRACE(e);
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			VE_TRACE(e);
		}

		while (m_Running)
		{
			if (VkCommandBuffer commandBuffer = m_Renderer->BeginFrame())
			{
				m_Renderer->BeginSwapChainRendererPass(commandBuffer);
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(commandBuffer);
				m_Renderer->EndSwapChainRendererPass(commandBuffer);
				m_Renderer->EndFrame();

				m_Window->OnUpdate();
			}
		}
	}

	bool App::OnWindowClose(const WindowCloseEvent& e)
	{
		vkDeviceWaitIdle(m_Device->GetDevice());
		for (Layer* layer : m_LayerStack)
			layer->OnDetach();
		m_Running = false;
		return true;
	}
}