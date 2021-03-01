#include "RenderBackend.h"

#include <volk.h>

class VulkanSwapChain;

class VulkanBackend : public iface::RenderBackend {
    // Implementing RenderBackend interface
public:
    void Initialize() override final;
    void Render() override final;

private:
    void CreateInstance();

private:
    VkInstance m_instance;
    VulkanSwapChain* m_swapChain;
};