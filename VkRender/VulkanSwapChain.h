#pragma once

#include <volk.h>

#include <vector>

#include "render_utils.h"

class VulkanSurface;

class VulkanSwapChain{
public:
    VulkanSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VulkanSurface *surface, uint32_t familyIndex, VkFormat format, uint32_t width, uint32_t height, VkRenderPass renderPass, const uint32_t bufferSize);
    ~VulkanSwapChain();

    void InitializeSwapChain();
    void ResizeOnNeed(uint32_t &w, uint32_t &h);
    void Destroy(VkSwapchainKHR swapChain, std::vector<VkImageView> &imageViews, std::vector<VkFramebuffer> &framebuffers) const;

    VkSwapchainKHR& GetSwapChain();
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    VkImage& GetImage(size_t idx);
    VkFramebuffer& GetFB(size_t idx);
    uint32_t GetImageCount() const;
    VkImageMemoryBarrier CreateImageBarrier(uint32_t imageindex, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout) const;
    uint32_t AcquireNextImage(VkSemaphore acquireSemaphore);
    void BindRenderStartBarrier(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void BindRenderEndBarrier(VkCommandBuffer commandBuffer, uint32_t imageIndex);
private:
    void CreateSwapChain(VkSwapchainKHR oldSwapChain = nullptr);
    VkFramebuffer CreateFramebuffer(VkImageView imageView) const;
    VkImageView CreateImageView(VkImage image) const;
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() const;
private:
    VkSwapchainKHR m_swapChain;

	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	std::vector<VkFramebuffer> m_framebuffers;

	uint32_t m_width;
    uint32_t m_height;
	uint32_t m_imageCount;
    VkFormat m_format;

    VkPhysicalDevice r_physicalDevice;
    VkDevice r_device;
    VulkanSurface * r_surface;
    uint32_t r_familyIndex;
    VkRenderPass r_renderPass;
    
    const uint32_t mr_bufferSize;
};