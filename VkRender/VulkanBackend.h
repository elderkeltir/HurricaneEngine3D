#include "RenderBackend.h"

#include <volk.h>

class VulkanSwapChain;
class VulkanSurface;
class VulkanPipelineCollection;
class VulkanShaderManager;
class VulkanCommandQueueDispatcher;

class VulkanBackend : public iface::RenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend();
    // Implementing RenderBackend interface
public:
    void Initialize(const char * rootFolder) override final;
    void Render() override final;

private:
    void CreateInstance();
    VkPhysicalDevice PickPhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices) const;
    void CreateDevice();

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;

    VulkanSwapChain* m_swapChain;
    VulkanSurface * m_surface;
    VulkanPipelineCollection * m_pipelineCollection;
    VulkanShaderManager * m_shaderMgr;
    VulkanCommandQueueDispatcher * m_cmdQueueDispatcher;

    char m_rootFolder[255];
};