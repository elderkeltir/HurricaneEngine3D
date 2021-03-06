#pragma once

#include "interfaces/RenderMesh.h"
#include "render_utils.h"

#include <volk.h>

#include<vector>

class VulkanMemoryManager;

class VulkanMesh : public iface::RenderMesh{
public:
    struct Vertex
    {
        float vx, vy, vz;
        float nx, ny, nz;
        float tu, tv;
    };

    VulkanMesh();
    ~VulkanMesh();
    void Initialize(const char *path, VulkanMemoryManager * memoryMgr, VkDevice device);
    void Render(VkCommandBuffer commandBuffer);
private:
    bool ParseObj(const char* path); // for now let it be here, move to imported/mesh manager instead at some future pass

    std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

    BufferPtr m_vBuffPtr;
    BufferPtr m_iBuffPtr;

    VkDevice r_device;
};