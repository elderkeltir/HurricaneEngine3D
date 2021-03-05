#include "VulkanCommandQueueDispatcher.h"
#include "VulkanSwapChain.h"

#include <vector>

VulkanCommandQueueDispatcher::VulkanCommandQueueDispatcher(VkPhysicalDevice physicalDevice){
	r_physicalDevice = physicalDevice;
	m_queues.resize(QT_size);

	uint32_t gFamilyIdx = TestFamilQueueyIndex(VK_QUEUE_GRAPHICS_BIT, r_physicalDevice);
	assert(gFamilyIdx != VK_QUEUE_FAMILY_IGNORED);
	m_queues[QT_graphics].familyQueueIndex = gFamilyIdx;
}

VulkanCommandQueueDispatcher::~VulkanCommandQueueDispatcher(){
	for (GQueue & queue : m_queues){
		vkDestroyCommandPool(r_device, queue.commandPool, 0);
	}
	
	vkDestroySemaphore(r_device, m_releaseSemaphore, 0);
	vkDestroySemaphore(r_device, m_acquireSemaphore, 0);
}

void VulkanCommandQueueDispatcher::Initialize(VkDevice device, const uint32_t bufferSize){
    r_device = device;
	const uint32_t cmdBuffersCount = bufferSize;
	
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
	m_queues[QT_graphics].commandBuffers.resize(cmdBuffersCount);
	for (size_t i = 0u; i < cmdBuffersCount; i++){
		VkCommandBuffer gCommandBuffer = CreateCommandBuffer(m_queues[QT_graphics].commandPool);
		assert(gCommandBuffer);
		m_queues[QT_graphics].commandBuffers[i] = gCommandBuffer;
	}
}

VulkanCommandQueueDispatcher::GQueue VulkanCommandQueueDispatcher::GetQueue(QueueType type) const{
	assert(type == QT_graphics);
	return m_queues[type];
}

VkCommandBuffer VulkanCommandQueueDispatcher::GetCommandBuffer(QueueType type, uint32_t commandBufferIndex) const{
	return m_queues[QT_graphics].commandBuffers[commandBufferIndex];
}

VkSemaphore VulkanCommandQueueDispatcher::GetAquireSemaphore() const{
	return m_acquireSemaphore;
}

VkSemaphore VulkanCommandQueueDispatcher::GetReleaseSemaphore() const{
	return m_releaseSemaphore;
}

void VulkanCommandQueueDispatcher::ResetCommandPool(QueueType type){
	VK_CHECK(vkResetCommandPool(r_device, m_queues[type].commandPool, 0));
}

void VulkanCommandQueueDispatcher::BeginCommandBuffer(QueueType type, uint32_t commandBufferIndex){
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(m_queues[type].commandBuffers[commandBufferIndex], &beginInfo));
}

void VulkanCommandQueueDispatcher::EndCommandBuffer(QueueType type, uint32_t commandBufferIndex){
	VK_CHECK(vkEndCommandBuffer(m_queues[type].commandBuffers[commandBufferIndex]));
}

void VulkanCommandQueueDispatcher::SubmitQueue(QueueType type, uint32_t commandBufferIndex){
	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_acquireSemaphore;
	submitInfo.pWaitDstStageMask = &submitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_queues[type].commandBuffers[commandBufferIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_releaseSemaphore;

	VK_CHECK(vkQueueSubmit(m_queues[type].queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void VulkanCommandQueueDispatcher::PresentQueue(QueueType type, uint32_t imageIndex, VulkanSwapChain * swapChain){
	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_releaseSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain->GetSwapChain();
	presentInfo.pImageIndices = &imageIndex;

	VK_CHECK(vkQueuePresentKHR(m_queues[type].queue, &presentInfo));
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

VkCommandPool VulkanCommandQueueDispatcher::CreateCommandPool(uint32_t familyIndex) const {
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	createInfo.queueFamilyIndex = familyIndex;

	VkCommandPool commandPool = 0;
	VK_CHECK(vkCreateCommandPool(r_device, &createInfo, 0, &commandPool));

	return commandPool;
}

VkCommandBuffer VulkanCommandQueueDispatcher::CreateCommandBuffer(VkCommandPool commandPool) const{
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = 0;
	VK_CHECK(vkAllocateCommandBuffers(r_device, &allocateInfo, &commandBuffer));

	return commandBuffer;
}