#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Renderer/Instance.h"
#include "Renderer/SwapChain.h"
#include <GLFW/glfw3.h>

namespace VoxelicousEngine 
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "VoxelicousEngine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual VkSurfaceKHR GetVkSurfaceKHR() const = 0;
		virtual GLFWwindow* GetGLFWwindow() const = 0;
		//virtual SwapChain& GetSwapChain() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(Instance& instance, const WindowProps& props = WindowProps());
	};

}