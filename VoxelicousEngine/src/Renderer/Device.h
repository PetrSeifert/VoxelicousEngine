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
        bool IsComplete() const { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
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

        VkCommandPool GetCommandPool() const { return m_CommandPool; }
        VkDevice GetDevice() const { return m_Device; }
        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }

        SwapChainSupportDetails GetSwapChainSupport(const VkSurfaceKHR surface) const
        {
            return QuerySwapChainSupport(m_PhysicalDevice, surface);
        }

        QueueFamilyIndices FindPhysicalQueueFamilies(const VkSurfaceKHR surface) const
        {
            return FindQueueFamilies(m_PhysicalDevice, surface);
        }

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        VkFormat FindSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        // Buffer Helper Functions
        void CreateBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory) const;
        VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
        void CopyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;

        void CreateImageWithInfo(
            const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory) const;

        VkPhysicalDeviceProperties Properties;

    private:
        void PickPhysicalDevice(VkSurfaceKHR surface);
        void CreateLogicalDevice(VkSurfaceKHR surface);
        void CreateCommandPool(VkSurfaceKHR surface);

        // helper functions
        bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) const;
        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkCommandPool m_CommandPool;

        VkInstance m_VkInstance;
        VkDevice m_Device;
        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;

        const std::vector<const char*> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}
