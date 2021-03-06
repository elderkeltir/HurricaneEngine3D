#pragma once

#include "interfaces/RenderBackend.h"

#include <vector>
#include <cinttypes>

#define DEFINE_HANDLE(object) typedef struct object##_T* object;

class VulkanSwapChain;
class VulkanSurface;
class VulkanPipelineCollection;
class VulkanShaderManager;
class VulkanCommandQueueDispatcher;
class VulkanMesh;
class VulkanMemoryManager;

DEFINE_HANDLE(VkPhysicalDevice)
DEFINE_HANDLE(VkInstance)
DEFINE_HANDLE(VkDevice)

class VulkanBackend : public iface::RenderBackend {
public:
    VulkanBackend();
    ~VulkanBackend() override;
    // Implementing RenderBackend interface
public:
    void Initialize(const char * rootFolder) override final;
    void Render() override final;
    bool IsRunning() override final;

private:
    void CreateInstance();
    VkPhysicalDevice PickPhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices) const;
    void CreateDevice();

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    const uint32_t m_bufferSize;

    VulkanSwapChain* m_swapChain;
    VulkanSurface * m_surface;
    VulkanPipelineCollection * m_pipelineCollection;
    VulkanShaderManager * m_shaderMgr;
    VulkanCommandQueueDispatcher * m_cmdQueueDispatcher;
    VulkanMemoryManager * m_memoryMgr;

    char m_rootFolder[255];
    std::vector<VulkanMesh> m_meshes; // TODO: move to mesh manager/pool in a future pass
};