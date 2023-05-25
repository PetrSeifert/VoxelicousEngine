#pragma once

#include <vulkan/vulkan.h>

namespace VoxelicousEngine
{
	class Instance
	{
#ifdef NDEBUG
		const bool enableValidationLayers = true;
#else
		const bool enableValidationLayers = false;
#endif

	public:
		Instance();

		~Instance();

		inline VkInstance Get() { return m_Instance; }

	private:
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;

		void SetupDebugMessenger();

		bool CheckValidationLayerSupport();

		void HasGlfwRequiredInstanceExtensions();

		std::vector<const char*> GetRequiredExtensions();

		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	};
}