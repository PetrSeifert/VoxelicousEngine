#pragma once

#include "Core/Window.h"

namespace VoxelicousEngine
{
    class WindowsWindow final : public Window
    {
    public:
        explicit WindowsWindow(Instance& instance, const WindowSpecification& props);
        ~WindowsWindow() override;

        void OnUpdate() override;

        unsigned int GetWidth() const override { return m_Data.Width; }
        unsigned int GetHeight() const override { return m_Data.Height; }

        VkSurfaceKHR GetVkSurfaceKHR() const override { return m_VkSurface; }
        GLFWwindow* GetGLFW_Window() const override { return m_Window; }

        //inline SwapChain& GetSwapChain() const override { return ; }

        // Window attributes
        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
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
