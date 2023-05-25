#pragma once

#include "Core/Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "Events/Event.h"
#include "Events/AppEvent.h"

namespace VoxelicousEngine
{
	class Renderer
	{
	public:

		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		inline VkRenderPass GetSwapChainRenderPass() const { return m_SwapChain->GetRenderPass(); }
		inline float GetAspectRatio() const { return m_SwapChain->ExtentAspectRatio(); }
		inline bool IsFrameInProgress() const { return m_IsFrameStarted; }

		inline SwapChain& GetSwapChain() const { return *m_SwapChain; }

		inline VkCommandBuffer GetCurrentCommandBuffer() const
		{
			assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_CommandBuffers[m_CurrentFrameIndex];
		}

		inline int GetFrameIndex() const
		{
			assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
			return m_CurrentFrameIndex;
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginSwapChainRendererPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRendererPass(VkCommandBuffer commandBuffer);

	private:
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void DrawFrame();
		void RecreateSwapChain();

		void OnWindowResized();

		Window& m_Window;
		Device& m_Device;
		std::unique_ptr<SwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		uint32_t m_CurrentImageIndex{ 0 };
		int m_CurrentFrameIndex{ 0 };
		bool m_IsFrameStarted{ false };
		bool m_WasWindowResized{ false };
	};
}