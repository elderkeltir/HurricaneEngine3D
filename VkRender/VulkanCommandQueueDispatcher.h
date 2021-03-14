#pragma once

#include "interfaces/RenderCommandQueueDispatcher.h"
#include "render_utils.h"

#include <volk.h>

#include <vector>

class VulkanSwapChain;

class VulkanCommandQueueDispatcher : public iface::RenderCommandQueueDispatcher{
public:
    enum QueueType{
        QT_graphics = 0,
        QT_transfer = 1 // TODO: Not used for now. it's not 100% win technique, only win on discrete gpu. will implement later
    };
    struct GQueue{
        uint32_t familyQueueIndex;
        VkQueue queue;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers; // Use L * T + N pools. (L = the number of buffered frames, T = the number of threads which record command buffers, N = extra pools for secondary command buffers).
        std::vector<VkFence> cmdBufferFences;
        GQueue() : familyQueueIndex(~0u), queue(nullptr), commandPool(nullptr) {}
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
    void CopyBuffer(const BufferPtr &srcBuffer, const BufferPtr &dstBuffer) const;
    void CopyBufferToImage(const BufferPtr &buffer, VkImage image, uint32_t width, uint32_t height) const;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;

    static uint32_t TestFamilQueueyIndex(VkPhysicalDevice physicalDevice, uint8_t queueFlags /*VkQueueFlagBits*/, uint8_t queueNotFlags = 0 /*VkQueueFlagBits*/);
private:
    VkSemaphore CreateSemaphore() const;
    VkCommandPool CreateCommandPool(uint32_t familyIndex) const;
    VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool) const;
    VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;
    
    std::vector<GQueue> m_queues;
    VkSemaphore m_acquireSemaphore;
    VkSemaphore m_releaseSemaphore;
    
    VkPhysicalDevice r_physicalDevice;
    VkDevice r_device;
};