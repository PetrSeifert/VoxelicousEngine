#include "vepch.h"
#include "WindowsWindow.h"

#include "Core/App.h"
#include "Events/AppEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

namespace VoxelicousEngine
{
    Window* Window::Create(Instance& instance, const WindowProps& props)
    {
        return new WindowsWindow(instance, props);
    }

    WindowsWindow::WindowsWindow(Instance& instance, const WindowProps& props) : m_Instance(instance)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        VE_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        m_Window = glfwCreateWindow(static_cast<int>(props.Width), static_cast<int>(props.Height), m_Data.Title.c_str(),
                                    nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window,
                                  [](GLFWwindow* window, const int width, const int height)
                                  {
                                      WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
                                      data.Width = width;
                                      data.Height = height;

                                      WindowResizeEvent event(width, height);
                                      data.EventCallback(event);
                                  });

        glfwSetWindowCloseCallback(m_Window,
                                   [](GLFWwindow* window)
                                   {
                                       const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(
                                           window));
                                       WindowCloseEvent event;
                                       data.EventCallback(event);
                                   });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, const int key, int scancode, const int action, int mods)
        {
            const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
            case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
            case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
            case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, const int button, const int action, int mods)
        {
            const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
            case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window,
                              [](GLFWwindow* window, const double xOffset, const double yOffset)
                              {
                                  const WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

                                  MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
                                  data.EventCallback(event);
                              });

        glfwSetCursorPosCallback(m_Window,
                                 [](GLFWwindow* window, const double xPos, const double yPos)
                                 {
                                     const WindowData& data = *static_cast<WindowData*>(
                                         glfwGetWindowUserPointer(window));

                                     MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
                                     data.EventCallback(event);
                                 });


        if (glfwCreateWindowSurface(instance.Get(), m_Window, nullptr, &m_VkSurface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    WindowsWindow::~WindowsWindow()
    {
        glfwDestroyWindow(m_Window);
        vkDestroySurfaceKHR(m_Instance.Get(), m_VkSurface, nullptr);
    }

    void WindowsWindow::OnUpdate()
    {
        glfwPollEvents();
    }

    void WindowsWindow::SetVSync(const bool enabled)
    {
        // TODO: Vulkan VSYNC
        m_Data.VSync = enabled;
    }

    bool WindowsWindow::IsVSync() const
    {
        return m_Data.VSync;
    }
}
