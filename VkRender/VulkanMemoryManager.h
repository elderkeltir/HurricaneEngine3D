#pragma once

#include "render_utils.h"

#include <volk.h>

#include <vector>

class VulkanMemoryManager{
public:
    enum BufferUsageType{
            BUT_vertex_buffer = 1 << 0,
            BUT_index_buffer = 1 << 1,
            BUT_uniform_buffer = 1 << 2
    };
    enum MemoryPropertyFlag{
        MPF_host_visible = 1 << 0,
        MPF_device_local = 1 << 1,
        MPF_host_coherent = 1 << 2
    };
    struct BufferSet{
        VkBuffer aggregatedBuffer;
        VkDeviceMemory aggregatedMemory;
        uint32_t bufferType; /*BufferUsageType*/
        uint32_t memoryType; /*MemoryPropertyFlag*/
        uint32_t nextFreeSlice;
        BufferSet() : aggregatedBuffer(nullptr), aggregatedMemory(nullptr), bufferType(0u), memoryType(0u), nextFreeSlice(0u) {}
    };

    void Initialize(VkPhysicalDevice physicalDevice, VkDevice device);
    BufferPtr AllocateBuffer(size_t size, BufferUsageType usageType); //TODO: deallocate // track empty spots using same offset+size maybe for start

    VulkanMemoryManager();
    ~VulkanMemoryManager();
private:
    uint32_t SelectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags);
    std::vector<BufferSet> m_buffers;

    VkPhysicalDevice r_physicalDevice;
    VkDevice r_device;
};