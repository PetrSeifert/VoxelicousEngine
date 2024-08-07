#include "vepch.h"
#include "App.h"

#include "Log.h"
#include "Renderer/SwapChain.h"
#include "GLFW/glfw3.h"

#include "imgui_internal.h"

#include <filesystem>
#include <ranges>

#include "Assert.h"


extern bool g_ApplicationRunning;
extern ImGuiContext* GImGui;

namespace VoxelicousEngine
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        VE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }
    
    #define BIND_EVENT_FN(x) std::bind(&App::x, this, std::placeholders::_1)

    #define BIND_EVENT_FN(fn) std::bind(&Application::##fn, this, std::placeholders::_1)

    App* App::s_Instance = nullptr;

    static std::thread::id s_MainThreadID;

    App::Application(const AppSpecification& specification)
        : m_Specification(specification), m_RenderThread(specification.CoreThreadingPolicy), m_AppSettings("App.hsettings")
    {
        VE_CORE_INFO("Started app!");
        s_Instance = this;
        s_MainThreadID = std::this_thread::get_id();

        m_AppSettings.Deserialize();

        m_RenderThread.Run();

        // Set working directory here
        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);
        
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
        
		m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
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

    void App::Close()
    {
        m_Running = false;
    }

    void App::SubmitToMainThread(const std::function<void()>& function)
    {
        std::scoped_lock lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }
    
    void App::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

        for (const auto& it : std::ranges::reverse_view(m_LayerStack))
        {
            if (e.Handled) 
                break;

            it->OnEvent(e);
        }
    }

    void App::Run()
    {
        const WindowResizeEvent e(m_Specification.Width, m_Specification.Height);
        
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
            float time = Time::GetTime();
            TimeStep timeStep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            ExecuteMainThreadQueue();

            if (!m_Minimized)
            {
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(timeStep);

                m_ImGuiLayer->Begin();
                {
                    for (Layer* layer : m_LayerStack)
                        layer->OnImGuiRender();
                }
                m_ImGuiLayer->End();
            }
            
            m_Window->OnUpdate();
        }
    }

    bool App::OnWindowClose(const WindowCloseEvent& e)
    {
        vkDeviceWaitIdle(m_Device->GetDevice());
        m_Running = false;
        return true;
    }

    bool App::OnWindowResize(const WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;

        return false;
    }

    void App::ExecuteMainThreadQueue()
    {
        std::scoped_lock lock(m_MainThreadQueueMutex);

        for (auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }
}
