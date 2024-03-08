#include "vepch.h"
#include "Instance.h"

#include "Core/App.h"

#include <GLFW/glfw3.h>

namespace VoxelicousEngine
{
    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(
        const VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT")); func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
        const VkInstance instance,
        const VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT")); func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    Instance::Instance()
    {
        if (m_EnableValidationLayers && !CheckValidationLayerSupport())
        {
            std::cout << "validation layers requested, but not available!" << '\n';
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Voxelicous engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        const auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (m_EnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
        {
            VE_CORE_ERROR("Failed to create instance!");
        }

        HasGlfwRequiredInstanceExtensions();
        SetupDebugMessenger();
    }

    Instance::~Instance()
    {
        if (m_EnableValidationLayers)
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

        vkDestroyInstance(m_Instance, nullptr);
    }

    void Instance::SetupDebugMessenger()
    {
        if (!m_EnableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);
        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
        {
            VE_CORE_ERROR("Failed to set up debug messenger!");
        }
    }

    void Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr; // Optional
    }

    bool Instance::CheckValidationLayerSupport() const
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_ValidationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    void Instance::HasGlfwRequiredInstanceExtensions() const
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        VE_CORE_INFO("available extensions:");
        std::unordered_set<std::string> available;
        for (const auto& extension : extensions)
        {
            VE_CORE_INFO(std::format("\t {}", extension.extensionName));
            available.emplace(extension.extensionName);
        }

        std::cout << "required extensions:" << '\n';
        for (const auto requiredExtensions = GetRequiredExtensions(); const auto& required : requiredExtensions)
        {
            VE_CORE_INFO(std::format("\t {}", required));
            if (!available.contains(required))
            {
                VE_CORE_ERROR("Missing required glfw extension!");
            }
        }
    }

    std::vector<const char*> Instance::GetRequiredExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (m_EnableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }
}
