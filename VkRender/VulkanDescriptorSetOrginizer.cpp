#include "VulkanDescriptorSetOrginizer.h"


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
	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(poolSetup.unixormBufferNumber);
	
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(poolSetup.maxSetNumber);

	vkCreateDescriptorPool(r_device, &poolInfo, nullptr, &m_descriptorPool);
}

VulkanDescriptorSetOrginizer::DescriptorPoolSetup VulkanDescriptorSetOrginizer::SetupDescriptorPoolConfig() const{
    // predict necessary pool size here for the whole application
    DescriptorPoolSetup poolSetup;
    poolSetup.unixormBufferNumber = 3;
    poolSetup.maxSetNumber = 3;

    return poolSetup;
}