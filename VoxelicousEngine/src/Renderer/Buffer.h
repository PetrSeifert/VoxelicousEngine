#pragma once

#include "Device.h"

namespace VoxelicousEngine
{
    class Buffer
    {
    public:
        Buffer(
            Device& device,
            VkDeviceSize instanceSize,
            uint32_t instanceCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        /**
         * Copies the specified data to the mapped buffer. REQUIRES the buffer to be mapped and host-visible.
         *
         * @param data Pointer to the data to copy
         * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
         * @param offset (Optional) Byte offset from beginning of mapped region
         */
        void WriteToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        /**
         * Uploads data to this buffer (potentially device-local) using a temporary host-visible staging buffer.
         *
         * @param data Pointer to the data to copy
         * @param size Size of the data to copy.
         * @param offset Byte offset in the destination buffer to copy to.
         */
        void UploadDataViaStaging(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

        VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        void WriteToIndex(const void* data, int index) const;
        VkResult FlushIndex(int index) const;
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index) const;
        VkResult InvalidateIndex(int index) const;

        VkBuffer GetBuffer() const { return m_Buffer; }
        void* GetMappedMemory() const { return m_Mapped; }
        uint32_t GetInstanceCount() const { return m_InstanceCount; }
        VkDeviceSize GetInstanceSize() const { return m_InstanceSize; }
        VkDeviceSize GetAlignmentSize() const { return m_InstanceSize; }
        VkBufferUsageFlags GetUsageFlags() const { return m_UsageFlags; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
        VkDeviceSize GetBufferSize() const { return m_BufferSize; }

    private:
        static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        Device& m_Device;
        void* m_Mapped = nullptr;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;

        VkDeviceSize m_BufferSize;
        uint32_t m_InstanceCount;
        VkDeviceSize m_InstanceSize;
        VkDeviceSize m_AlignmentSize;
        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;
    };
}
