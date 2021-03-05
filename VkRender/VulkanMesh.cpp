#include "VulkanMesh.h"
#include "VulkanMemoryManager.h"

#include "meshoptimizer.h"
#pragma warning(disable : 4996)
#define FAST_OBJ_IMPLEMENTATION
#include "../extern/fast_obj.h"


VulkanMesh::VulkanMesh(){

}

VulkanMesh::~VulkanMesh(){

}

void VulkanMesh::Initialize(const char *path, VulkanMemoryManager * memoryMgr, VkDevice device){
    assert(ParseObj(path));
    r_device = device;
    
    assert(memoryMgr);
    m_vBuffPtr = memoryMgr->AllocateBuffer(m_vertices.size() * sizeof(Vertex), VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer);
    m_vBuffPtr.Validate();

    m_iBuffPtr = memoryMgr->AllocateBuffer(m_indices.size() * sizeof(uint32_t), VulkanMemoryManager::BufferUsageType::BUT_index_buffer);
    m_iBuffPtr.Validate();

    void* vData = 0;
	VK_CHECK(vkMapMemory(r_device, m_vBuffPtr.memoryRef, m_vBuffPtr.offset, m_vBuffPtr.size, 0, &vData));
    assert(vData);
    memcpy(vData, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    vkUnmapMemory(r_device, m_vBuffPtr.memoryRef);

    void* iData = 0;
	VK_CHECK(vkMapMemory(r_device, m_iBuffPtr.memoryRef, m_iBuffPtr.offset, m_iBuffPtr.size, 0, &iData));
    assert(iData);
    memcpy(iData, m_indices.data(), m_indices.size() * sizeof(uint32_t));
    vkUnmapMemory(r_device, m_iBuffPtr.memoryRef);
}

void VulkanMesh::Render(VkCommandBuffer commandBuffer){
	VkDeviceSize dummyOffset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vBuffPtr.bufferRef, &m_vBuffPtr.offset);
	vkCmdBindIndexBuffer(commandBuffer, m_iBuffPtr.bufferRef, m_iBuffPtr.offset, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
}

bool VulkanMesh::ParseObj(const char* path){
	fastObjMesh* obj = fast_obj_read(path);
	if (!obj)
	{
		printf("Error loading %s: file not found\n", path);
		return false;
	}

	size_t total_indices = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
		total_indices += 3 * (obj->face_vertices[i] - 2);

	std::vector<Vertex> vertices(total_indices);

	size_t vertex_offset = 0;
	size_t index_offset = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
	{
		for (unsigned int j = 0; j < obj->face_vertices[i]; ++j)
		{
			fastObjIndex gi = obj->indices[index_offset + j];

			Vertex v =
			{
				obj->positions[gi.p * 3 + 0],
				obj->positions[gi.p * 3 + 1],
				obj->positions[gi.p * 3 + 2],
				obj->normals[gi.n * 3 + 0],
				obj->normals[gi.n * 3 + 1],
				obj->normals[gi.n * 3 + 2],
				obj->texcoords[gi.t * 2 + 0],
				obj->texcoords[gi.t * 2 + 1],
			};

			// triangulate polygon on the fly; offset-3 is always the first polygon vertex
			if (j >= 3)
			{
				vertices[vertex_offset + 0] = vertices[vertex_offset - 3];
				vertices[vertex_offset + 1] = vertices[vertex_offset - 1];
				vertex_offset += 2;
			}

			vertices[vertex_offset] = v;
			vertex_offset++;
		}

		index_offset += obj->face_vertices[i];
	}

	fast_obj_destroy(obj);

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));

	m_indices.resize(total_indices);
	meshopt_remapIndexBuffer(&m_indices[0], NULL, total_indices, &remap[0]);

	m_vertices.resize(total_vertices);
	meshopt_remapVertexBuffer(&m_vertices[0], &vertices[0], total_indices, sizeof(Vertex), &remap[0]);

	return true;
}