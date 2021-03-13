#include "VulkanCommandQueueDispatcher.h"
#include "VulkanSwapChain.h"

#include <vector>

VulkanCommandQueueDispatcher::VulkanCommandQueueDispatcher(VkPhysicalDevice physicalDevice){
	r_physicalDevice = physicalDevice;
	m_queues.resize(2u);

	// Graphics queue
	uint32_t gFamilyIdx = TestFamilQueueyIndex(r_physicalDevice, VK_QUEUE_GRAPHICS_BIT);
	assert(gFamilyIdx != VK_QUEUE_FAMILY_IGNORED);
	m_queues[QT_graphics].familyQueueIndex = gFamilyIdx;

	// Transfer queue
	uint32_t tFamilyIdx = TestFamilQueueyIndex(r_physicalDevice, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT);
	assert(tFamilyIdx != VK_QUEUE_FAMILY_IGNORED);
	m_queues[QT_transfer].familyQueueIndex = tFamilyIdx;
}

VulkanCommandQueueDispatcher::~VulkanCommandQueueDispatcher(){
	for (GQueue & queue : m_queues){
		if (!queue.cmdBufferFences.empty()){
			VK_CHECK(vkWaitForFences(r_device, queue.cmdBufferFences.size(), &queue.cmdBufferFences[0], 1u, ~0u));
		}
		if (queue.commandPool){
			vkDestroyCommandPool(r_device, queue.commandPool, 0);
		}
		for (VkFence &fence : queue.cmdBufferFences){
			vkDestroyFence(r_device, fence, 0);
		}
	}
	
	vkDestroySemaphore(r_device, m_releaseSemaphore, 0);
	vkDestroySemaphore(r_device, m_acquireSemaphore, 0);
}

void VulkanCommandQueueDispatcher::Initialize(VkDevice device, const uint32_t bufferSize){
    r_device = device;
	const uint32_t cmdBuffersCount = bufferSize;
	
	// create queues
	VkQueue gQueue;
	vkGetDeviceQueue(device, m_queues[QT_graphics].familyQueueIndex, 0, &gQueue);
	assert(gQueue);
	m_queues[QT_graphics].queue = gQueue;

	VkQueue tQueue;
	vkGetDeviceQueue(device, m_queues[QT_transfer].familyQueueIndex, 0, &tQueue);
	assert(tQueue);
	m_queues[QT_transfer].queue = tQueue;

	// semaphores
	m_acquireSemaphore = CreateSemaphore();
	assert(m_acquireSemaphore);
	m_releaseSemaphore = CreateSemaphore();
	assert(m_releaseSemaphore);

	// cmd pools
	VkCommandPool gCmdPool = CreateCommandPool(m_queues[QT_graphics].familyQueueIndex);
	assert(gCmdPool);
	m_queues[QT_graphics].commandPool = gCmdPool;

	VkCommandPool tCmdPool = CreateCommandPool(m_queues[QT_transfer].familyQueueIndex);
	assert(tCmdPool);
	m_queues[QT_transfer].commandPool = tCmdPool;

	// cmd buffers
	m_queues[QT_graphics].commandBuffers.resize(cmdBuffersCount);
	m_queues[QT_graphics].cmdBufferFences.resize(cmdBuffersCount);
	for (size_t i = 0u; i < cmdBuffersCount; i++){
		VkCommandBuffer gCommandBuffer = CreateCommandBuffer(m_queues[QT_graphics].commandPool);
		assert(gCommandBuffer);
		m_queues[QT_graphics].commandBuffers[i] = gCommandBuffer;

		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(r_device, &fenceInfo, 0, &m_queues[QT_graphics].cmdBufferFences[i]);
	}
}

VulkanCommandQueueDispatcher::GQueue VulkanCommandQueueDispatcher::GetQueue(QueueType type) const{
	assert(type < m_queues.size());
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

	VK_CHECK(vkWaitForFences(r_device, 1, &m_queues[type].cmdBufferFences[commandBufferIndex], 0u, ~0u));
	VK_CHECK(vkResetFences(r_device, 1, &m_queues[type].cmdBufferFences[commandBufferIndex]));

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

	VK_CHECK(vkQueueSubmit(m_queues[type].queue, 1, &submitInfo, m_queues[type].cmdBufferFences[commandBufferIndex]));
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

void VulkanCommandQueueDispatcher::CopyBuffer(const BufferPtr &srcBuffer, const BufferPtr &dstBuffer) const{
	VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_queues[QT_graphics].commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(r_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcBuffer.offset;
	copyRegion.dstOffset = dstBuffer.offset;
	copyRegion.size = srcBuffer.size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer.bufferRef, dstBuffer.bufferRef, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_queues[QT_graphics].queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queues[QT_graphics].queue);

	vkFreeCommandBuffers(r_device, m_queues[QT_graphics].commandPool, 1, &commandBuffer);
}

uint32_t VulkanCommandQueueDispatcher::TestFamilQueueyIndex(VkPhysicalDevice physicalDevice, uint8_t queueFlags, uint8_t queueNotFlags){
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, 0);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queues.data());

	for (uint32_t i = 0; i < queueCount; ++i)
		if ((queues[i].queueFlags & queueFlags) && !(queues[i].queueFlags & queueNotFlags))
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
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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