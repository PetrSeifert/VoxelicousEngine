#include "vepch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "Platform/Vulkan/ImGuiVulkanRenderer.h"
#include "Platform/GLFW/ImGuiGlfwRenderer.h"

namespace VoxelicousEngine {

	/*struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		glm::vec3 lightPosition{ -5.f, -6.f, 0.f };
		alignas(16) glm::vec4 lightColor{ 1.f };
	};*/

	ImGuiLayer::ImGuiLayer(Renderer& renderer, Device& device, DescriptorPool& globalPool) : Layer("ImGuiLayer"), m_Renderer(renderer), m_Device(device), m_GlobalPool(globalPool)
	{
	}

	ImGuiLayer::~ImGuiLayer() = default;

	void ImGuiLayer::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(m_Window.GetGLFWwindow(), true);
		App& app = App::Get();
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = app.GetInstance().Get();
		initInfo.PhysicalDevice = m_Device.GetPhysicalDevice();
		initInfo.Device = m_Device.GetDevice();
		initInfo.DescriptorPool = m_GlobalPool.GetDescriptorPool();
		initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfo.Queue = m_Device.GetGraphicsQueue();
		initInfo.MinImageCount = 2;

		ImGui_ImplVulkan_Init(&initInfo, m_Renderer.GetSwapChainRenderPass());
		VkCommandBuffer commandBuffer = m_Renderer.BeginFrame();
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

	void ImGuiLayer::OnUpdate(VkCommandBuffer commandBuffer)
	{
		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - m_CurrentTime).count();
		m_CurrentTime = newTime;

		float aspect = m_Renderer.GetAspectRatio();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, 0);
	}

	void ImGuiLayer::OnEvent(Event& event)
	{

	}

}