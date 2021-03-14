#include "VulkanMemoryManager.h"

static uint32_t gBufferSize = 1024u * 1024u *8u;
static uint32_t gLocalBufferSize = 1024u * 1024u *2u;

uint32_t GetVulkanBufferUsageFlags(uint32_t myFlags){
    uint32_t vulkanFlags = 0u;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer) vulkanFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_index_buffer) vulkanFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_uniform_buffer) vulkanFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_transfer_src) vulkanFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_transfer_dst) vulkanFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_sampled) vulkanFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (myFlags & VulkanMemoryManager::BufferUsageType::BUT_depth) vulkanFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    return vulkanFlags;
}

uint32_t GetMyBufferUsageFlags(uint32_t vulkanFlags){
    uint32_t myFlags = 0u;
    if (vulkanFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer;
    if (vulkanFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_index_buffer;
    if (vulkanFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_uniform_buffer;
    if (vulkanFlags & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_transfer_src;
    if (vulkanFlags & VK_BUFFER_USAGE_TRANSFER_DST_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_transfer_dst;
    if (vulkanFlags & VK_IMAGE_USAGE_SAMPLED_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_sampled;
    if (vulkanFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) myFlags |= VulkanMemoryManager::BufferUsageType::BUT_depth;

    return myFlags;
}

uint32_t GetVulkanMemoryPropertyFlags(uint32_t myFlags){
    uint32_t vulkanFlags = 0u;
    if (myFlags & VulkanMemoryManager::MemoryPropertyFlag::MPF_device_local) vulkanFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (myFlags & VulkanMemoryManager::MemoryPropertyFlag::MPF_host_visible) vulkanFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (myFlags & VulkanMemoryManager::MemoryPropertyFlag::MPF_host_coherent) vulkanFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return vulkanFlags;
}

uint32_t GetMyMemoryPropertyFlags(uint32_t vulkanFlags){
    uint32_t myFlags = 0u;
    if (vulkanFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) myFlags |= VulkanMemoryManager::MemoryPropertyFlag::MPF_device_local;
    if (vulkanFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) myFlags |= VulkanMemoryManager::MemoryPropertyFlag::MPF_host_visible;
    if (vulkanFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) myFlags |= VulkanMemoryManager::MemoryPropertyFlag::MPF_host_coherent;

    return myFlags;
}

void VulkanMemoryManager::Initialize(VkPhysicalDevice physicalDevice, VkDevice device){
    r_physicalDevice = physicalDevice;
    r_device = device;
    {
        const uint32_t buffertype = BufferUsageType::BUT_index_buffer | BufferUsageType::BUT_uniform_buffer | BufferUsageType::BUT_transfer_src;
        const uint32_t memoryType = MemoryPropertyFlag::MPF_host_visible | MemoryPropertyFlag::MPF_host_coherent;
        BufferSet bufferSet =  CreateBufferSet(buffertype, memoryType, gBufferSize);
        m_buffers.push_back(bufferSet);
    }
    {
        const uint32_t buffertype = BufferUsageType::BUT_vertex_buffer | BufferUsageType::BUT_transfer_dst;
        const uint32_t memoryType = MemoryPropertyFlag::MPF_device_local;
        BufferSet bufferSet =  CreateBufferSet(buffertype, memoryType, gLocalBufferSize);
        m_buffers.push_back(bufferSet);
    }
}

BufferPtr VulkanMemoryManager::AllocateBuffer(size_t size, uint32_t usageType){
    // TODO: deallocation
    BufferPtr buffPtr;
    for (BufferSet &bufferSet : m_buffers){
        if (bufferSet.bufferType & GetVulkanBufferUsageFlags(usageType)){
            assert(bufferSet.size - bufferSet.nextFreeSlice > size);

            uint32_t actual_size = (size / 16) * 16; // TODO: props.limits.minUniformBufferOffsetAlignment is 16 now

            buffPtr.bufferRef = bufferSet.aggregatedBuffer;
            buffPtr.memoryRef = bufferSet.aggregatedMemory;
            buffPtr.offset = bufferSet.nextFreeSlice;
            buffPtr.size = actual_size;
            buffPtr.usageType = usageType;

            bufferSet.nextFreeSlice += actual_size;

            break;
        }
    }

    assert(buffPtr.size);

    return buffPtr;
}

VulkanMemoryManager::VulkanMemoryManager() :
    r_physicalDevice(nullptr),
    r_device(nullptr)
{
}

VulkanMemoryManager::~VulkanMemoryManager(){
    for (BufferSet &bufferSet : m_buffers){
        vkFreeMemory(r_device, bufferSet.aggregatedMemory, 0);
	    vkDestroyBuffer(r_device, bufferSet.aggregatedBuffer, 0);
    }
}

uint32_t VulkanMemoryManager::SelectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) const
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
		if ((memoryTypeBits & (1 << i)) != 0 && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
			return i;

	assert(!"No compatible memory type found");
	return ~0u;
}

VulkanMemoryManager::BufferSet VulkanMemoryManager::CreateBufferSet(uint32_t bufferType, uint32_t memory_type, uint32_t size) const{
    BufferSet bufferSet;

    bufferSet.size = size;
    bufferSet.bufferType = GetVulkanBufferUsageFlags(bufferType);
    bufferSet.memoryType = GetVulkanMemoryPropertyFlags(memory_type);
    
    VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.size = size;
	createInfo.usage = bufferSet.bufferType;

	VK_CHECK(vkCreateBuffer(r_device, &createInfo, 0, &bufferSet.aggregatedBuffer));
    assert(bufferSet.aggregatedBuffer);

    VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(r_device, bufferSet.aggregatedBuffer, &memoryRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(r_physicalDevice, &memoryProperties);

	uint32_t memoryTypeIndex = SelectMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, bufferSet.memoryType);
	assert(memoryTypeIndex != ~0u);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VK_CHECK(vkAllocateMemory(r_device, &allocateInfo, 0, &bufferSet.aggregatedMemory));
    assert(bufferSet.aggregatedMemory);
	VK_CHECK(vkBindBufferMemory(r_device, bufferSet.aggregatedBuffer, bufferSet.aggregatedMemory, 0));

    return bufferSet;
}

ImagePtr VulkanMemoryManager::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, uint32_t usage, VkImageAspectFlags aspect, bool samplerNeeded) {
    ImagePtr imagePtr;
    imagePtr.usageType = GetVulkanBufferUsageFlags(usage);

    // image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imagePtr.usageType;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    VK_CHECK(vkCreateImage(r_device, &imageInfo, nullptr, &imagePtr.imageRef));    

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(r_device, imagePtr.imageRef, &memRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(r_physicalDevice, &memoryProperties);
    imagePtr.size = memRequirements.size;

    // memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = SelectMemoryType(memoryProperties, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(r_device, &allocInfo, nullptr, &imagePtr.memoryRef));

    vkBindImageMemory(r_device, imagePtr.imageRef, imagePtr.memoryRef, 0);

    // image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = imagePtr.imageRef;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(r_device, &viewInfo, nullptr, &imagePtr.imageView));

    // sampler
    if (samplerNeeded) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(r_physicalDevice, &properties); // TODO: move to backend. get all the necessary props there and pass backend as 1st arg to every constructor

        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        VK_CHECK(vkCreateSampler(r_device, &samplerInfo, nullptr, &imagePtr.sampler));
    }

    return imagePtr;
}