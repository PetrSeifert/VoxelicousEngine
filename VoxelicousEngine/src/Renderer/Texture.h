#pragma once

#include "Device.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>

namespace VoxelicousEngine {

    class Texture {
    public:
        // Constructor for creating a texture specifically for CPU upload target
        Texture(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) = default;
        Texture& operator=(Texture&&) = default;

        VkImageView GetImageView() const { return m_ImageView; }
        VkSampler GetSampler() const { return m_Sampler; }
        VkImageLayout GetImageLayout() const { return m_ImageLayout; }
        VkDescriptorImageInfo GetDescriptorInfo() const;
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        VkFormat GetFormat() const { return m_Format; }
        Device& GetDevice() { return m_Device; }

        // Transition image layout
        void TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

        // Update texture from CPU data (requires staging buffer)
        void UpdateFromBuffer(const void* srcData, VkDeviceSize bufferSize);

    private:
        void CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags);
        void CreateSampler();

        Device& m_Device;
        VkImage m_Image = VK_NULL_HANDLE;
        VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;
        VkFormat m_Format;
        VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_MipLevels = 1; // Keep it simple for now
    };

} // namespace VoxelicousEngine 