#include "VulkanDescriptorSetOrginizer.h"

#include <vector>

#define DESCRIPTOR_SIZE 32

VulkanDescriptorSetOrginizer::VulkanDescriptorSetOrginizer() :
    m_descriptorPool(nullptr)
{

}

VulkanDescriptorSetOrginizer::~VulkanDescriptorSetOrginizer(){
    vkDestroyDescriptorPool(r_device, m_descriptorPool, nullptr);
}

void VulkanDescriptorSetOrginizer::Initialize(VkDevice device){
    r_device = device;
    DescriptorPoolSetup poolSetup = SetupDescriptorPoolConfig();
    CreateDescriptorPool(poolSetup);
}

const VkDescriptorPool & VulkanDescriptorSetOrginizer::GetDescriptorPool() const{
    return m_descriptorPool;
}

void VulkanDescriptorSetOrginizer::CreateDescriptorPool(DescriptorPoolSetup poolSetup){
    std::vector<VkDescriptorPoolSize> poolSizes(2);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(poolSetup.unixormBufferNumber);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = poolSetup.combinedImageSampleNumber;
	
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(poolSetup.maxSetNumber);

	vkCreateDescriptorPool(r_device, &poolInfo, nullptr, &m_descriptorPool);
}

VulkanDescriptorSetOrginizer::DescriptorPoolSetup VulkanDescriptorSetOrginizer::SetupDescriptorPoolConfig() const{
    // predict necessary pool size here for the whole application
    DescriptorPoolSetup poolSetup;
    poolSetup.unixormBufferNumber = 3 * DESCRIPTOR_SIZE;
    poolSetup.combinedImageSampleNumber = 3 * DESCRIPTOR_SIZE;
    poolSetup.maxSetNumber = 6 * DESCRIPTOR_SIZE;

    return poolSetup;
}