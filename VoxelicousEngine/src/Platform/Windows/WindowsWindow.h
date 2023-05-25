#pragma once

#include "Core/Window.h"

namespace VoxelicousEngine 
{
	class WindowsWindow : public Window
	{
	public:
		explicit WindowsWindow(Instance& instance, const WindowProps& props);
		~WindowsWindow() final;

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		inline VkSurfaceKHR GetVkSurfaceKHR() const override { return m_VkSurface; }
		inline GLFWwindow* GetGLFWwindow() const override { return m_Window; }

		//inline SwapChain& GetSwapChain() const override { return ; }

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
	private:
		GLFWwindow* m_Window{};
		VkSurfaceKHR m_VkSurface;
		//std::unique_ptr<SwapChain> m_SwapChain;

		Instance& m_Instance;

		struct WindowData
		{
			std::string Title;
			unsigned int Width;
			unsigned int Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}