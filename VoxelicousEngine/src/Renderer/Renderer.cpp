#include "vepch.h"
#include "Renderer.h"
#include "Core/Log.h"
#include "GLFW/glfw3.h"

namespace VoxelicousEngine
{

	Renderer::Renderer(Window& window, Device& device) : m_Window{ window }, m_Device{ device }
	{
		//bug??
		m_CurrentFrameIndex = 0;
		RecreateSwapChain();
		CreateCommandBuffers();
	}

	Renderer::~Renderer() { FreeCommandBuffers(); }

	void Renderer::OnWindowResized()
	{
		m_WasWindowResized = true;
	}

	void Renderer::RecreateSwapChain()
	{
		auto extent = VkExtent2D{ m_Window.GetWidth(), m_Window.GetHeight() };
		while (extent.width == 0 || extent.height == 0)
		{
			extent = VkExtent2D{ m_Window.GetWidth(), m_Window.GetHeight() };
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device.GetDevice());

		if (m_SwapChain == nullptr)
		{
			m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, m_Window.GetVkSurfaceKHR());
		}
		else {
			std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
			m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, m_Window.GetVkSurfaceKHR(), oldSwapChain);

			if (!oldSwapChain->CompareSwapFormats(*m_SwapChain.get()))
			{
				VE_CORE_ERROR("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void Renderer::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Device.GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			VE_CORE_ERROR("Failed to allocate command buffers!");
		}
	}

	void Renderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_Device.GetDevice(),
			m_Device.GetCommandPool(),
			static_cast<uint32_t>(m_CommandBuffers.size()),
			m_CommandBuffers.data());

		m_CommandBuffers.clear();
	}

	VkCommandBuffer Renderer::BeginFrame()
	{
		VE_CORE_ASSERT(!m_IsFrameStarted && "Can't call beginFrame while already in progress!");

		auto result = m_SwapChain->AcquireNextImage(&m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			VE_CORE_ERROR("Failed to acquire swap chain image!");
		}

		m_IsFrameStarted = true;

		auto commandBuffer = GetCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			VE_CORE_ERROR("Failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void Renderer::EndFrame() {
		VE_CORE_ASSERT(m_IsFrameStarted && "Can't call endFrame while frame is not in progress!");
		auto commandBuffer = GetCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			VE_CORE_ERROR("Failed to record command buffer!");
		}

		if (auto result = m_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex); result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_WasWindowResized)
		{
			m_WasWindowResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			VE_CORE_ERROR("Failed to present swap chain image!");
		}

		m_IsFrameStarted = false;
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::BeginSwapChainRendererPass(VkCommandBuffer commandBuffer)
	{
		VE_CORE_ASSERT(m_IsFrameStarted && "Can't call beginSwapChainRendererPass if frame is not in progress!");
		VE_CORE_ASSERT(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
		renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(m_CurrentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
		viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_SwapChain->GetSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::EndSwapChainRendererPass(VkCommandBuffer commandBuffer)
	{
		VE_CORE_ASSERT(m_IsFrameStarted && "Can't call endSwapChainRendererPass if frame is not in progress!");
		VE_CORE_ASSERT(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!");

		vkCmdEndRenderPass(commandBuffer);
	}
}