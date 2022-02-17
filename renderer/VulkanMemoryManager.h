#pragma once

#include "render_utils.h"

#include <volk.h>

#include <vector>

class VulkanMemoryManager{
public:
    enum BufferUsageType{
            BUT_vertex_buffer = 1 << 0,
            BUT_index_buffer = 1 << 1,
            BUT_uniform_buffer = 1 << 2,
            BUT_transfer_src = 1 << 3,
            BUT_transfer_dst = 1 << 4,
            BUT_sampled = 1 << 5,
            BUT_depth = 1 << 6,
    };
    enum MemoryPropertyFlag{
        MPF_host_visible = 1 << 0,
        MPF_device_local = 1 << 1,
        MPF_host_coherent = 1 << 2
    };
    struct BufferSet{
        size_t size;
        VkBuffer aggregatedBuffer;
        VkDeviceMemory aggregatedMemory;
        uint32_t bufferType; /*BufferUsageType*/
        uint32_t memoryType; /*MemoryPropertyFlag*/
        uint32_t nextFreeSlice;
        BufferSet() : size(0u), aggregatedBuffer(nullptr), aggregatedMemory(nullptr), bufferType(0u), memoryType(0u), nextFreeSlice(0u) {}
    };

    void Initialize(VkPhysicalDevice physicalDevice, VkDevice device);
    BufferPtr AllocateBuffer(size_t size, uint32_t usageType); //TODO: deallocate // track empty spots using same offset+size maybe for start
    ImagePtr AllocateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, uint32_t usage, VkImageAspectFlags aspect, bool samplerNeeded);

    VulkanMemoryManager();
    ~VulkanMemoryManager();
private:
    uint32_t SelectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) const;
    BufferSet CreateBufferSet(uint32_t bufferType, uint32_t memory_type, uint32_t size) const;
    
    std::vector<BufferSet> m_buffers;

    VkPhysicalDevice r_physicalDevice;
    VkDevice r_device;
};