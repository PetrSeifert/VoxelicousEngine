#include "vepch.h"
#include "Texture.h"
#include "Buffer.h" // For staging buffer
#include "Core/Core.h"
#include "Core/Log.h"

namespace VoxelicousEngine {

    Texture::Texture(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
        : m_Device(device), m_Width(width), m_Height(height), m_Format(format)
    {
        VE_CORE_ASSERT(width > 0 && height > 0, "Texture width and height must be greater than 0");
        
        // Ensure necessary usage flags are present
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // Needed for copying from staging buffer
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;      // Needed for sampling in shaders (e.g., full-screen quad)

        CreateImage(format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        CreateImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
        CreateSampler();
        
        // Initial layout transition (optional, can be done externally when needed)
        // TransitionLayout(?, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    Texture::~Texture() {
        // Use vkDestroySampler/ImageView/Image and vkFreeMemory
        // Ensure these are called *before* the device is destroyed
        vkDestroySampler(m_Device.GetDevice(), m_Sampler, nullptr);
        vkDestroyImageView(m_Device.GetDevice(), m_ImageView, nullptr);
        vkDestroyImage(m_Device.GetDevice(), m_Image, nullptr);
        vkFreeMemory(m_Device.GetDevice(), m_ImageMemory, nullptr);
    }

    void Texture::CreateImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_Width;
        imageInfo.extent.height = m_Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = m_MipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_Device.GetDevice(), &imageInfo, nullptr, &m_Image) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create image!");
            throw std::runtime_error("Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device.GetDevice(), m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Device.FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device.GetDevice(), &allocInfo, nullptr, &m_ImageMemory) != VK_SUCCESS) {
             VE_CORE_FATAL("Failed to allocate image memory!");
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(m_Device.GetDevice(), m_Image, m_ImageMemory, 0);
    }

    void Texture::CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_MipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create texture image view!");
            throw std::runtime_error("Failed to create texture image view!");
        }
    }

    void Texture::CreateSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        // Use nearest neighbor filtering for voxel lookups or raytraced results
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE; // Typically disabled for this use case
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(m_MipLevels);

        if (vkCreateSampler(m_Device.GetDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
            VE_CORE_FATAL("Failed to create texture sampler!");
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }
    
    VkDescriptorImageInfo Texture::GetDescriptorInfo() const {
        return VkDescriptorImageInfo{
            m_Sampler, 
            m_ImageView, 
            m_ImageLayout // Use the current layout when getting info
        };
    }

    void Texture::TransitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_MipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        // Define access masks and pipeline stages based on layout transition
        // (Simplified version - needs refinement based on actual usage)
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or compute shader stage if used there
        } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or compute shader stage
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Written by compute
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Read by fragment (fullscreen quad)
            sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Was read by fragment
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Will be written by compute
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        } else {
             // Keep warning for genuinely unexpected transitions (like the 5->1, 1->5)
             VE_CORE_WARN("Unsupported layout transition specified in Texture::TransitionLayout! From {} to {}", oldLayout, newLayout);
             barrier.srcAccessMask = 0; // Default safe values
             barrier.dstAccessMask = 0;
             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
             destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
             // Consider throwing an error for unsupported transitions
             // throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        
        m_ImageLayout = newLayout; // Update the stored layout
    }

    void Texture::UpdateFromBuffer(const void* srcData, VkDeviceSize bufferSize) {
        // 1. Create Staging Buffer
        Buffer stagingBuffer{
            m_Device,
            bufferSize,
            1, // instance count
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        // 2. Map memory and copy data
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)srcData);
        // stagingBuffer.Unmap(); // Unmap often implicitly handled by destructor or flush
        stagingBuffer.Flush(); // Ensure data is visible to device

        // 3. Execute copy command
        VkCommandBuffer commandBuffer = m_Device.BeginSingleTimeCommands();

        // Transition image layout to TRANSFER_DST_OPTIMAL
        TransitionLayout(commandBuffer, m_ImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy buffer to image
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0; // Tightly packed
        region.bufferImageHeight = 0; // Tightly packed
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {m_Width, m_Height, 1};

        vkCmdCopyBufferToImage(
            commandBuffer,
            stagingBuffer.GetBuffer(),
            m_Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        // Transition image layout back to SHADER_READ_ONLY_OPTIMAL (or desired final layout)
        TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_Device.EndSingleTimeCommands(commandBuffer);

        // Staging buffer is destroyed automatically when it goes out of scope
    }

} // namespace VoxelicousEngine 