#pragma once

#include "interfaces/RenderMesh.h"
#include "interfaces/RenderPipelineCollection.h"
#include "render_utils.h"

#include <volk.h>
#include <glm/mat4x4.hpp>

#include<vector>

class VulkanMemoryManager;
class VulkanCommandQueueDispatcher;
class VulkanPipelineCollection;

class VulkanMesh : public iface::RenderMesh{
public:
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
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
    void Initialize(const char *path,
                    const char *texturePath, 
                    VulkanMemoryManager * memoryMgr, 
                    VulkanCommandQueueDispatcher * queueDispatcher, 
                    VkDevice device, VkDescriptorPool descriptorPool, 
                    iface::RenderPipelineCollection::PipelineType pipelineType, 
                    VulkanPipelineCollection *pipelineCollection, 
                    uint32_t imageCount); // TODO: time to get a structure for intialization(vulkan-like?)
    void Render(float dt, VkCommandBuffer commandBuffer, uint32_t imageIndex);
    iface::RenderPipelineCollection::PipelineType GetPipelineType() const;

    VulkanMesh(VulkanMesh&&);
    VulkanMesh& operator=(VulkanMesh&&);

    VulkanMesh(const VulkanMesh&) = delete;
    VulkanMesh& operator=(const VulkanMesh&) = delete;
private:
    bool ParseObj(const char* path); // for now let it be here, move to imported/mesh manager instead at some future pass
    void UpdateUniformBuffers(float dt, uint32_t imageIndex);
    void CreateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t imageCount);
    void UpdateDescriptorSets();
    void LoadTexture(VulkanMemoryManager * memoryMgr, VulkanCommandQueueDispatcher * queueDispatcher, const char *texturePath);

    std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

    float m_color[4]; // TODO: switch to material?.. check PBR code to not fck up 

    BufferPtr m_vBuffPtr;
    BufferPtr m_iBuffPtr;
    BufferPtr m_vsBuffPtr;
    BufferPtr m_tsBuffPtr;
    ImagePtr m_imagePtr;

    std::vector<BufferPtr> m_uniformBuffers;
    std::vector<VkDescriptorSet> m_descriptorSets;
    iface::RenderPipelineCollection::PipelineType m_pipelineType;

    VulkanPipelineCollection *r_pipelineCollection;
    VkDevice r_device;
};