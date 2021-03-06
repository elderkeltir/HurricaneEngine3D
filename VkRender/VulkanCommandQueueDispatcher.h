#pragma once

#include "interfaces/RenderCommandQueueDispatcher.h"
#include "render_utils.h"

#include <volk.h>

#include <vector>

// Modify QueueFamilyIndices and findQueueFamilies to explicitly look for a queue family with the VK_QUEUE_TRANSFER_BIT bit, but not the VK_QUEUE_GRAPHICS_BIT.
// Modify createLogicalDevice to request a handle to the transfer queue
// Create a second command pool for command buffers that are submitted on the transfer queue family
// Change the sharingMode of resources to be VK_SHARING_MODE_CONCURRENT and specify both the graphics and transfer queue families
// Submit any transfer commands like vkCmdCopyBuffer (which we'll be using in this chapter) to the transfer queue instead of the graphics queue

class VulkanSwapChain;

class VulkanCommandQueueDispatcher : public iface::RenderCommandQueueDispatcher{
public:
    enum QueueType{
        QT_graphics,
        QT_compute,
        QT_transfer,
        QT_size
    };
    struct GQueue{
        uint32_t familyQueueIndex;
        VkQueue queue;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers; // Use L * T + N pools. (L = the number of buffered frames, T = the number of threads which record command buffers, N = extra pools for secondary command buffers).
        std::vector<VkFence> cmdBufferFences;
        GQueue() : familyQueueIndex(0), queue(nullptr), commandPool(nullptr) {}
    };
public:
    VulkanCommandQueueDispatcher(VkPhysicalDevice physicalDevice);
    ~VulkanCommandQueueDispatcher();
    void Initialize(VkDevice device, const uint32_t bufferSize);
    GQueue GetQueue(QueueType type) const;
    VkCommandBuffer GetCommandBuffer(QueueType type, uint32_t commandBufferIndex) const;
    VkSemaphore GetAquireSemaphore() const;
    VkSemaphore GetReleaseSemaphore() const;
    void ResetCommandPool(QueueType type);
    void BeginCommandBuffer(QueueType type, uint32_t commandBufferIndex);
    void EndCommandBuffer(QueueType type, uint32_t commandBufferIndex);
    void SubmitQueue(QueueType type, uint32_t commandBufferIndex);
    void PresentQueue(QueueType type, uint32_t imageIndex, VulkanSwapChain * swapChain);

    static uint32_t TestFamilQueueyIndex(uint8_t queueFlags /*VkQueueFlagBits*/, VkPhysicalDevice physicalDevice);
private:
    VkSemaphore CreateSemaphore() const;
    VkCommandPool CreateCommandPool(uint32_t familyIndex) const;
    VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool) const;
    
    std::vector<GQueue> m_queues;
    VkSemaphore m_acquireSemaphore;
    VkSemaphore m_releaseSemaphore;
    
    VkPhysicalDevice r_physicalDevice;
    VkDevice r_device;
};