#include "VulkanCommandQueueDispatcher.h"

#include <vector>

VulkanCommandQueueDispatcher::VulkanCommandQueueDispatcher(VkPhysicalDevice physicalDevice){
	r_physicalDevice = physicalDevice;
	m_queues.resize(QT_size);

	uint32_t gFamilyIdx = TestFamilQueueyIndex(VK_QUEUE_GRAPHICS_BIT, r_physicalDevice);
	assert(gFamilyIdx = VK_QUEUE_FAMILY_IGNORED);
	m_queues[QT_graphics].familyQueueIndex = gFamilyIdx;
}

void VulkanCommandQueueDispatcher::Initialize(VkDevice device){
    r_device = device;
	
	VkQueue gQueue;
	vkGetDeviceQueue(device, m_queues[QT_graphics].familyQueueIndex, 0, &gQueue);
	assert(gQueue);

	m_queues[QT_graphics].queue = gQueue;

	// semaphores
	m_acquireSemaphore = CreateSemaphore();
	assert(m_acquireSemaphore);
	m_releaseSemaphore = CreateSemaphore();
	assert(m_releaseSemaphore);

	// cmd pools
	VkCommandPool gCmdPool = CreateCommandPool(m_queues[QT_graphics].familyQueueIndex);
	assert(gCmdPool);
	m_queues[QT_graphics].commandPool = gCmdPool;

	// cmd buffers
	VkCommandBuffer gCommandBuffer = CreateCommandBuffer(m_queues[QT_graphics].commandPool);
	assert(gCommandBuffer);
	m_queues[QT_graphics].commandBuffer = gCommandBuffer;
}

uint32_t VulkanCommandQueueDispatcher::GetFamilyQueueIndex(QueueType type) const{
	assert(type == QT_graphics);
	return m_queues[type].familyQueueIndex;
}

VkQueue VulkanCommandQueueDispatcher::GetQueue(QueueType type) const{
	assert(type == QT_graphics);
	return m_queues[type].queue;
}

VkSemaphore VulkanCommandQueueDispatcher::GetAquireSemaphore() const{
	return m_acquireSemaphore;
}

VkSemaphore VulkanCommandQueueDispatcher::GetReleaseSemaphore() const{
	return m_releaseSemaphore;
}

uint32_t VulkanCommandQueueDispatcher::TestFamilQueueyIndex(uint8_t queueFlags, VkPhysicalDevice physicalDevice){
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, 0);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queues.data());

	for (uint32_t i = 0; i < queueCount; ++i)
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			return i;

	return VK_QUEUE_FAMILY_IGNORED;
}

VkSemaphore VulkanCommandQueueDispatcher::CreateSemaphore() const {
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	VkSemaphore semaphore = 0;
	VK_CHECK(vkCreateSemaphore(r_device, &createInfo, 0, &semaphore));

	return semaphore;
}

VkCommandPool VulkanCommandQueueDispatcher::CreateCommandPool(uint32_t familyIndex){
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	createInfo.queueFamilyIndex = familyIndex;

	VkCommandPool commandPool = 0;
	VK_CHECK(vkCreateCommandPool(r_device, &createInfo, 0, &commandPool));
}

VkCommandBuffer VulkanCommandQueueDispatcher::CreateCommandBuffer(VkCommandPool commandPool){
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = 0;
	VK_CHECK(vkAllocateCommandBuffers(r_device, &allocateInfo, &commandBuffer));
}