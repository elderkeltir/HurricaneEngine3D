#pragma once

#include "render_utils.h"

#include <volk.h>

#include <vector>

class VulkanMemoryManager{
public:
    enum BufferUsageType{
            BUT_vertex_buffer,
            BUT_index_buffer,
            BUT_size
    };
    enum MemoryPropertyFlag{
        MPF_host_visible,
        MPF_device_local,
        MPF_host_coherent,
        MPF_size
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
uint32_t selectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
		if ((memoryTypeBits & (1 << i)) != 0 && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
			return i;

	assert(!"No compatible memory type found");
	return ~0u;
}
    //
    VkPhysicalDeviceMemoryProperties m_memoryProperties;
    std::vector<BufferSet> m_buffers;

    VkPhysicalDevice r_physicalDevice;
    VkDevice r_device;
};