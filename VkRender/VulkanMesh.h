#pragma once

#include "interfaces/RenderMesh.h"
#include "render_utils.h"

#include <volk.h>
#include <glm/mat4x4.hpp>

#include<vector>

class VulkanMemoryManager;
class VulkanCommandQueueDispatcher;

class VulkanMesh : public iface::RenderMesh{
public:
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    struct Vertex
    {
        float vx, vy, vz;
        float nx, ny, nz;
        float tu, tv;
    };
    struct DescriptorSet{ //TODO ?
    };
public:

    VulkanMesh();
    ~VulkanMesh();
    void Initialize(const char *path, VulkanMemoryManager * memoryMgr, VulkanCommandQueueDispatcher * queueDispatcher, VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t imageCount);
    void Render(float dt, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t imageIndex);
private:
    bool ParseObj(const char* path); // for now let it be here, move to imported/mesh manager instead at some future pass
    void UpdateUniformBuffers(float dt, uint32_t imageIndex);
    void CreateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t imageCount);
    void UpdateDescriptorSets();

    std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

    BufferPtr m_vBuffPtr;
    BufferPtr m_iBuffPtr;
    BufferPtr m_vsBuffPtr;

    std::vector<BufferPtr> m_uniformBuffers;
    std::vector<VkDescriptorSet> m_descriptorSets;

    VkDevice r_device;
};