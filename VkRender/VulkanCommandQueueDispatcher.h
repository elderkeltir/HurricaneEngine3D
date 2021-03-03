#include "RenderCommandQueueDispatcher.h"
#include "render_utils.h"

#include <volk.h>

#include <vector>

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
        VkCommandBuffer commandBuffer; // only one?
    };
public:
    VulkanCommandQueueDispatcher(VkPhysicalDevice physicalDevice);
    void Initialize(VkDevice device);
    uint32_t GetFamilyQueueIndex(QueueType type) const;
    VkQueue GetQueue(QueueType type) const;
    VkSemaphore GetAquireSemaphore() const;
    VkSemaphore GetReleaseSemaphore() const;

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