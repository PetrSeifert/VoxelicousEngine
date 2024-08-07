#include "vepch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "Platform/Vulkan/ImGuiVulkanRenderer.h"
#include "Platform/GLFW/ImGuiGlfwRenderer.h"

namespace VoxelicousEngine
{
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
        
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
    
        SetDarkThemeColors();

        App& app = App::Get();
        GLFWwindow* window = app.GetWindow().GetGLFW_Window();
        Device& device = app.GetDevice();
        DescriptorPool &globalPool = app.;
        
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = app.GetInstance().Get();
        initInfo.PhysicalDevice = device.GetPhysicalDevice();
        initInfo.Device = device.GetDevice();
        initInfo.DescriptorPool = m_GlobalPool.GetDescriptorPool();
        initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
        initInfo.Queue = device.GetGraphicsQueue();
        initInfo.MinImageCount = 2;

        ImGui_ImplVulkan_Init(&initInfo, m_Renderer.GetSwapChainRenderPass());
        const VkCommandBuffer commandBuffer = m_Renderer.BeginFrame();
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        m_Renderer.EndFrame();
        vkDeviceWaitIdle(m_Device.GetDevice());
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnUpdate(const VkCommandBuffer commandBuffer)
    {
        const auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(newTime - m_CurrentTime).count();
        m_CurrentTime = newTime;

        float aspect = m_Renderer.GetAspectRatio();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, nullptr);
    }

    void ImGuiLayer::OnEvent(Event& event)
    {
    }
}
