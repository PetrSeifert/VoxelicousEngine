#pragma once

#include "Device.h"

namespace VoxelicousEngine
{
    class Device;

    class SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(Device& deviceRef, VkExtent2D extent, VkSurfaceKHR surface);
        SwapChain(Device& deviceRef, VkExtent2D extent, VkSurfaceKHR surface, std::shared_ptr<SwapChain> previous);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        VkFramebuffer GetFrameBuffer(const int index) const { return m_SwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        VkImageView GetImageView(const int index) const { return m_SwapChainImageViews[index]; }
        size_t ImageCount() const { return m_SwapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
        uint32_t Width() const { return m_SwapChainExtent.width; }
        uint32_t Height() const { return m_SwapChainExtent.height; }

        float ExtentAspectRatio() const
        {
            return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
        }

        VkFormat FindDepthFormat() const;

        VkResult AcquireNextImage(uint32_t* imageIndex) const;
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex);

        bool CompareSwapFormats(const SwapChain& swapChain) const
        {
            return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat &&
                swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
        }

    private:
        void Init(VkSurfaceKHR surface);
        void CreateSwapChain(VkSurfaceKHR surface);
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        VkFormat m_SwapChainImageFormat;
        VkFormat m_SwapChainDepthFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemories;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        Device& m_Device;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain;
        std::shared_ptr<SwapChain> m_OldSwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame{0};
    };
}
