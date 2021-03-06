#pragma once

#include "interfaces/RenderPipelineCollection.h"

#include <volk.h>

#include <vector>

class VulkanShaderManager;
class VulkanSurface;

class VulkanPipelineCollection : public iface::RenderPipelineCollection{
public:
    struct VulkanPipelineSetup{
        VkPipelineLayout layout;
        VkPipeline pipeline;
        VkRenderPass renderPass;
    };
public:
    VulkanPipelineCollection();
    ~VulkanPipelineCollection();
    void Initialize(VkDevice device, VulkanShaderManager * shaderMgr, VulkanSurface * surface);
    const VulkanPipelineSetup& GetPipeline(PipelineType type) const;
    void BeginRenderPass(VkCommandBuffer commandBuffer, PipelineType type, VkFramebuffer framebuffer, uint32_t width, uint32_t height);
    void EndRenderPass(VkCommandBuffer commandBuffer);
    void BindPipeline(VkCommandBuffer commandBuffer, PipelineType type);

private:
    VkPipelineLayout CreatePipelineLayout(PipelineType type) const; // load form text files in future or binary files w\e
    VkPipeline CreateGraphicsPipeline(PipelineType type, VkPipelineLayout layout, VkRenderPass renderPass) const;
    VkRenderPass CreateRenderPass(PipelineType type) const;

    VkPipelineCache m_pipelineCache;
    std::vector<VulkanPipelineSetup> m_pipelines;

    VulkanShaderManager * r_shaderMgr;
    VulkanSurface * r_surface;
    VkDevice r_device;
};