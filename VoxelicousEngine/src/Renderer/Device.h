#pragma once

#include "vulkan/vulkan.h"

namespace VoxelicousEngine
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t GraphicsFamily;
		uint32_t PresentFamily;
		bool GraphicsFamilyHasValue = false;
		bool PresentFamilyHasValue = false;
		inline bool IsComplete() const { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
	};

	class Device
	{
	public:
		explicit Device(VkInstance vkInstance, VkSurfaceKHR surface);
		~Device();

		// Not copyable or movable
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(Device&&) = delete;

		inline VkCommandPool GetCommandPool() { return m_CommandPool; }
		inline VkDevice GetDevice() { return m_Device; }
		inline VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		inline VkQueue GetPresentQueue() { return m_PresentQueue; }
		inline VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }

		inline SwapChainSupportDetails GetSwapChainSupport(VkSurfaceKHR surface) { return QuerySwapChainSupport(m_PhysicalDevice, surface); }
		inline QueueFamilyIndices FindPhysicalQueueFamilies(VkSurfaceKHR surface) { return FindQueueFamilies(m_PhysicalDevice, surface); }

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		// Buffer Helper Functions
		void CreateBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(
			VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void CreateImageWithInfo(
			const VkImageCreateInfo& imageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory);

		VkPhysicalDeviceProperties properties;

	private:
		void PickPhysicalDevice(VkSurfaceKHR surface);
		void CreateLogicalDevice(VkSurfaceKHR surface);
		void CreateCommandPool(VkSurfaceKHR surface);

		// helper functions
		bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool;

		VkInstance m_VkInstance;
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}