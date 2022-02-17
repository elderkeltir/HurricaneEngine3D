#pragma once

#include <volk.h>

class VulkanDescriptorSetOrginizer{
public:
    VulkanDescriptorSetOrginizer();
    ~VulkanDescriptorSetOrginizer();
    void Initialize(VkDevice device);
    const VkDescriptorPool & GetDescriptorPool() const;
private:
    struct DescriptorPoolSetup{
        uint32_t uniformBufferNumber;
        uint32_t combinedImageSampleNumber;

        uint32_t maxSetNumber;
    };
    void CreateDescriptorPool(DescriptorPoolSetup poolSetup);
    DescriptorPoolSetup SetupDescriptorPoolConfig() const;
    VkDescriptorPool m_descriptorPool;

    VkDevice r_device;
};