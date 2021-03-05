#include "VulkanSwapChain.h"

#include "VulkanSurface.h"


VulkanSwapChain::VulkanSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VulkanSurface *surface, uint32_t familyIndex, VkFormat format, uint32_t width, uint32_t height, VkRenderPass renderPass, const uint32_t bufferSize) : 
    m_swapChain(nullptr),
	r_physicalDevice(physicalDevice),
	r_device(device),
	r_surface(surface),
	r_familyIndex(familyIndex),
	m_format(format),
	m_width(width),
	m_height(height),
	r_renderPass(renderPass),
	mr_bufferSize(bufferSize)
{

}

VulkanSwapChain::~VulkanSwapChain(){
	Destroy(m_swapChain, m_imageViews, m_framebuffers);
}

void VulkanSwapChain::InitializeSwapChain(){
	CreateSwapChain();
	assert(m_swapChain);

	VK_CHECK(vkGetSwapchainImagesKHR(r_device, m_swapChain, &m_imageCount, 0));

	m_images.resize(m_imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(r_device, m_swapChain, &m_imageCount, m_images.data()));


	m_imageViews.resize(m_imageCount);
	for (uint32_t i = 0; i < m_imageCount; ++i)
	{
		m_imageViews[i] = CreateImageView(m_images[i]);
		assert(m_imageViews[i]);
	}

	m_framebuffers.resize(m_imageCount);
	for (uint32_t i = 0; i < m_imageCount; ++i)
	{
		m_framebuffers[i] = CreateFramebuffer(m_imageViews[i]);
		assert(m_framebuffers[i]);
	}
}

void VulkanSwapChain::ResizeOnNeed(){
	VkSurfaceCapabilitiesKHR surfaceCaps = GetSurfaceCapabilities();

	const uint32_t newWidth = surfaceCaps.currentExtent.width;
	const uint32_t newHeight = surfaceCaps.currentExtent.height;

	if (m_width == newWidth && m_height == newHeight){
		return;
	}

	VkSwapchainKHR swapChain = m_swapChain;
	std::vector<VkImageView> imageViews = m_imageViews;
	std::vector<VkFramebuffer> framebuffers = m_framebuffers;


	CreateSwapChain(swapChain);

	VK_CHECK(vkDeviceWaitIdle(r_device));

	Destroy(swapChain, imageViews, framebuffers);
}

void VulkanSwapChain::Destroy(VkSwapchainKHR swapChain, std::vector<VkImageView> &imageViews, std::vector<VkFramebuffer> &framebuffers) const{
	for (uint32_t i = 0; i < framebuffers.size(); ++i)
		vkDestroyFramebuffer(r_device, framebuffers[i], 0);

	for (uint32_t i = 0; i < imageViews.size(); ++i)
		vkDestroyImageView(r_device, imageViews[i], 0);

	vkDestroySwapchainKHR(r_device, swapChain, 0);
}

uint32_t VulkanSwapChain::AcquireNextImage(VkSemaphore acquireSemaphore){
	uint32_t imageIndex = 0;
	VK_CHECK(vkAcquireNextImageKHR(r_device, m_swapChain, ~0ull, acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

	return imageIndex;
}

void VulkanSwapChain::BindRenderStartBarrier(VkCommandBuffer commandBuffer, uint32_t imageIndex){
	VkImageMemoryBarrier renderBeginBarrier = CreateImageBarrier(imageIndex, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderBeginBarrier);
}

void VulkanSwapChain::BindRenderEndBarrier(VkCommandBuffer commandBuffer, uint32_t imageIndex){
	VkImageMemoryBarrier renderEndBarrier = CreateImageBarrier(imageIndex, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderEndBarrier);

}

void VulkanSwapChain::CreateSwapChain(VkSwapchainKHR oldSwapChain){
	VkSurfaceCapabilitiesKHR surfaceCaps = GetSurfaceCapabilities();
	VkCompositeAlphaFlagBitsKHR surfaceComposite =
		(surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
		: (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
		? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
		: (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
		? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
		: VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = r_surface->Vk_surface();
	createInfo.minImageCount = std::max(mr_bufferSize, surfaceCaps.minImageCount);
	createInfo.imageFormat = m_format;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = m_width;
	createInfo.imageExtent.height = m_height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.queueFamilyIndexCount = 1;
	createInfo.pQueueFamilyIndices = &r_familyIndex;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = surfaceComposite;
	createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // use VK_PRESENT_MODE_FIFO_KHR for production, but hey, I want to benchmark fps :^)
	createInfo.oldSwapchain = oldSwapChain;

	VK_CHECK(vkCreateSwapchainKHR(r_device, &createInfo, 0, &m_swapChain));
}

VkFramebuffer VulkanSwapChain::CreateFramebuffer(VkImageView imageView) const
{
	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	createInfo.renderPass = r_renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &imageView;
	createInfo.width = m_width;
	createInfo.height = m_height;
	createInfo.layers = 1;

	VkFramebuffer framebuffer = 0;
	VK_CHECK(vkCreateFramebuffer(r_device, &createInfo, 0, &framebuffer));

	return framebuffer;
}

VkImageView VulkanSwapChain::CreateImageView(VkImage image) const
{
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = m_format;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView view = 0;
	VK_CHECK(vkCreateImageView(r_device, &createInfo, 0, &view));

	return view;
}

VkSurfaceCapabilitiesKHR VulkanSwapChain::GetSurfaceCapabilities() const{
	VkSurfaceCapabilitiesKHR surfaceCaps;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r_physicalDevice, r_surface->Vk_surface(), &surfaceCaps));

	return surfaceCaps;
}

VkSwapchainKHR& VulkanSwapChain::GetSwapChain(){
	return m_swapChain;
}

uint32_t VulkanSwapChain::GetWidth() const{
	return m_width;
}

uint32_t VulkanSwapChain::GetHeight() const{
	return m_height;
}

VkImage& VulkanSwapChain::GetImage(size_t idx){
	return m_images.at(idx);
}

VkFramebuffer& VulkanSwapChain::GetFB(size_t idx){
	return m_framebuffers.at(idx);
}

uint32_t VulkanSwapChain::GetImageCount() const{
	return m_imageCount;
}

VkImageMemoryBarrier VulkanSwapChain::CreateImageBarrier(uint32_t imageindex, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout) const{
	VkImageMemoryBarrier result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

	result.srcAccessMask = srcAccessMask;
	result.dstAccessMask = dstAccessMask;
	result.oldLayout = oldLayout;
	result.newLayout = newLayout;
	result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	result.image = m_images.at(imageindex);
	result.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	result.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	return result;
}