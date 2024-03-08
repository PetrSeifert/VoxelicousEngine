#pragma once

#include <vulkan/vulkan.h>

namespace VoxelicousEngine
{
    class Instance
    {
#ifdef NDEBUG
        const bool m_EnableValidationLayers = true;
#else
		const bool m_EnableValidationLayers = false;
#endif

    public:
        Instance();

        ~Instance();

        VkInstance Get() const { return m_Instance; }

    private:
        static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        void SetupDebugMessenger();

        bool CheckValidationLayerSupport() const;

        void HasGlfwRequiredInstanceExtensions() const;

        std::vector<const char*> GetRequiredExtensions() const;

        VkInstance m_Instance;
        VkDebugUtilsMessengerEXT m_DebugMessenger;

        std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    };
}
