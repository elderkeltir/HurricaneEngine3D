#include "VulkanMemoryManager.h"

static uint32_t gBufferSize = 1024u * 1024u *8u;

uint32_t GetVulkanBufferUsageFlags(uint32_t myFlags){
    uint32_t vulkanFlags = 0u;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer) vulkanFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_index_buffer) vulkanFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    return vulkanFlags;
}

uint32_t GetMyBufferUsageFlags(uint32_t vulkanFlags){
    uint32_t myFlags = 0u;
    if (vulkanFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer;
    if (vulkanFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_index_buffer;

    return myFlags;
}

uint32_t GetVulkanMemoryPropertyFlags(uint32_t myFlags){
    uint32_t vulkanFlags = 0u;
    if (myFlags & VulkanMemoryManager::MemoryPropertyFlag::MPF_device_local) vulkanFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (myFlags & VulkanMemoryManager::MemoryPropertyFlag::MPF_host_visible) vulkanFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (myFlags & VulkanMemoryManager::MemoryPropertyFlag::MPF_host_coherent) vulkanFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return vulkanFlags;
}

uint32_t GetMyMemoryPropertyFlags(uint32_t vulkanFlags){
    uint32_t myFlags = 0u;
    if (vulkanFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) myFlags |= VulkanMemoryManager::MemoryPropertyFlag::MPF_device_local;
    if (vulkanFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) myFlags |= VulkanMemoryManager::MemoryPropertyFlag::MPF_host_visible;
    if (vulkanFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) myFlags |= VulkanMemoryManager::MemoryPropertyFlag::MPF_host_coherent;

    return myFlags;
}

void VulkanMemoryManager::Initialize(VkPhysicalDevice physicalDevice, VkDevice device){
    r_physicalDevice = physicalDevice;
    r_device = device;
    BufferSet &bufferSet = m_buffers.front();

    bufferSet.bufferType = GetVulkanBufferUsageFlags(BufferUsageType::BUT_vertex_buffer | BufferUsageType::BUT_index_buffer);
    bufferSet.memoryType = GetVulkanMemoryPropertyFlags(MemoryPropertyFlag::MPF_host_visible | MemoryPropertyFlag::MPF_host_coherent);
    
    VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.size = gBufferSize;
	createInfo.usage = bufferSet.bufferType;

	VK_CHECK(vkCreateBuffer(r_device, &createInfo, 0, &bufferSet.aggregatedBuffer));
    assert(bufferSet.aggregatedBuffer);

    VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(r_device, bufferSet.aggregatedBuffer, &memoryRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(r_physicalDevice, &memoryProperties);

	uint32_t memoryTypeIndex = SelectMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, bufferSet.memoryType);
	assert(memoryTypeIndex != ~0u);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VK_CHECK(vkAllocateMemory(r_device, &allocateInfo, 0, &bufferSet.aggregatedMemory));
    assert(bufferSet.aggregatedMemory);
	VK_CHECK(vkBindBufferMemory(r_device, bufferSet.aggregatedBuffer, bufferSet.aggregatedMemory, 0));
}

BufferPtr VulkanMemoryManager::AllocateBuffer(size_t size, BufferUsageType usageType){
    // TODO: deallocation
    BufferPtr buffPtr;
    BufferSet &bufferSet = m_buffers.front();
    if (bufferSet.bufferType & GetVulkanBufferUsageFlags(usageType)){
        assert(gBufferSize - bufferSet.nextFreeSlice > size);

        buffPtr.bufferRef = bufferSet.aggregatedBuffer;
        buffPtr.memoryRef = bufferSet.aggregatedMemory;
        buffPtr.offset = bufferSet.nextFreeSlice;
        buffPtr.size = size;
        buffPtr.usageType = usageType;

        bufferSet.nextFreeSlice += size;
    }
    else{
        assert(false);
    }

    return buffPtr;
}

VulkanMemoryManager::VulkanMemoryManager() :
    r_physicalDevice(nullptr),
    r_device(nullptr)
{
    // 1 memory(host_visible | host_coherent) vs 1 buffer of combined type(vertex | index)
    // next pass is to use device_local for vertex and maybe index? + stagging buffer
    m_buffers.resize(1u); // TODO set for several during stagging buffer iml
}

VulkanMemoryManager::~VulkanMemoryManager(){
    assert(m_buffers.size() == 1u);
    BufferSet &bufferSet = m_buffers.front();
    vkFreeMemory(r_device, bufferSet.aggregatedMemory, 0);
	vkDestroyBuffer(r_device, bufferSet.aggregatedBuffer, 0);
}

uint32_t VulkanMemoryManager::SelectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
		if ((memoryTypeBits & (1 << i)) != 0 && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
			return i;

	assert(!"No compatible memory type found");
	return ~0u;
}