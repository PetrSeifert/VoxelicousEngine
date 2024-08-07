#pragma once

#include "Renderer/Renderer.h"

#include "Device.h"
#include "SwapChain.h"

struct GLFWwindow;

namespace VoxelicousEngine {

    class VulkanContext : public RendererContext
    {
    public:
        VulkanContext();
        virtual ~VulkanContext();

        virtual void Init() override;

        Ref<Device> GetDevice() { return m_Device; }

        static VkInstance GetInstance() { return s_VulkanInstance; }

        static Ref<VulkanContext> Get() { return Ref<VulkanContext>(Renderer::GetContext()); }
        static Ref<Device> GetCurrentDevice() { return Get()->GetDevice(); }
    private:
        // Devices
        Ref<PhysicalDevice> m_PhysicalDevice;
        Ref<Device> m_Device;

        // Vulkan instance
        inline static VkInstance s_VulkanInstance;
#if 0
        VkDebugReportCallbackEXT m_DebugReportCallback = VK_NULL_HANDLE;
#endif
        VkDebugUtilsMessengerEXT m_DebugUtilsMessenger = VK_NULL_HANDLE;
        VkPipelineCache m_PipelineCache = nullptr;

        SwapChain m_SwapChain;
    };
}