#pragma once
#include "SteelSightDevice.hpp"

namespace Voortman {
    class SteelSightBuffer final {
    public:
        SteelSightBuffer(SteelSightDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
        ~SteelSightBuffer();

        SteelSightBuffer(const SteelSightBuffer&) = delete;
        SteelSightBuffer& operator=(const SteelSightBuffer&) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void writeToIndex(void* data, int index);
        VkResult flushIndex(int index);
        VkDescriptorBufferInfo descriptorInfoForIndex(int index);
        VkResult invalidateIndex(int index);

        [[nodiscard]] const inline VkBuffer getBuffer()                           const noexcept { return buffer; }
        [[nodiscard]] const inline void* getMappedMemory()                        const noexcept { return mapped; }
        [[nodiscard]] const inline uint32_t getInstanceCount()                    const noexcept { return instanceCount; }
        [[nodiscard]] const inline VkDeviceSize getInstanceSize()                 const noexcept { return instanceSize; }
        [[nodiscard]] const inline VkDeviceSize getAlignmentSize()                const noexcept { return instanceSize; }
        [[nodiscard]] const inline VkBufferUsageFlags getUsageFlags()             const noexcept { return usageFlags; }
        [[nodiscard]] const inline VkMemoryPropertyFlags getMemoryPropertyFlags() const noexcept { return memoryPropertyFlags; }
        [[nodiscard]] const inline VkDeviceSize getBufferSize()                   const noexcept { return bufferSize; }

    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        SteelSightDevice& SSDevice;
        void* mapped = nullptr;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkDeviceSize bufferSize;
        uint32_t instanceCount;
        VkDeviceSize instanceSize;
        VkDeviceSize alignmentSize;
        VkBufferUsageFlags usageFlags;
        VkMemoryPropertyFlags memoryPropertyFlags;
    };
}