#pragma once

#include "interfaces/ShaderManager.h"
#include "interfaces/RenderPipelineCollection.h"

#include <volk.h>

struct VulkanShaderDB;

class VulkanShaderManager : public iface::ShaderManager{
public:
    VulkanShaderManager();
    ~VulkanShaderManager();
    void Initialize(VkDevice device, const char * folderPath);
    VkShaderModule GetShaderModule(iface::RenderPipelineCollection::PipelineType pipelineType, VulkanShaderManager::ShaderType shader_type, size_t index = 0u) const;
private:
    VkShaderModule LoadShader(const char* path);
    
    VulkanShaderDB * m_shaderDB;
    char m_folderPath[255];

    VkDevice r_device;
};