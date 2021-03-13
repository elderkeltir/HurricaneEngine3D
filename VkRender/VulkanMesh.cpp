#include "VulkanMesh.h"
#include "VulkanMemoryManager.h"

#include "meshoptimizer.h"
#pragma warning(disable : 4996)
#define FAST_OBJ_IMPLEMENTATION
#include "../extern/fast_obj.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>


VulkanMesh::VulkanMesh(){

}

VulkanMesh::~VulkanMesh(){

}

void VulkanMesh::Initialize(const char *path, VulkanMemoryManager * memoryMgr, VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t imageCount){
    assert(ParseObj(path));
    r_device = device;
    
	// Allocate buffers for vertex, index buffers
    assert(memoryMgr);
    m_vBuffPtr = memoryMgr->AllocateBuffer(m_vertices.size() * sizeof(Vertex), VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer);
    m_vBuffPtr.Validate();

    m_iBuffPtr = memoryMgr->AllocateBuffer(m_indices.size() * sizeof(uint32_t), VulkanMemoryManager::BufferUsageType::BUT_index_buffer);
    m_iBuffPtr.Validate();

	// load vertex, index data to memory
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

	// allocate uniform buffers
	m_uniformBuffers.resize(imageCount);
	for (uint32_t i = 0u; i < imageCount; i++){
		m_uniformBuffers[i] = memoryMgr->AllocateBuffer(sizeof(UniformBufferObject), VulkanMemoryManager::BufferUsageType::BUT_uniform_buffer);
    	m_uniformBuffers[i].Validate();
	}

	CreateDescriptorSets(descriptorPool, descriptorSetLayout, imageCount);
	UpdateDescriptorSets(); // TODO: maybe there is some reasons to add flexibility here?
}

void VulkanMesh::Render(float dt, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t imageIndex){
	UpdateUniformBuffers(dt, imageIndex);
	
	VkDeviceSize dummyOffset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vBuffPtr.bufferRef, &m_vBuffPtr.offset);
	vkCmdBindIndexBuffer(commandBuffer, m_iBuffPtr.bufferRef, m_iBuffPtr.offset, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_descriptorSets[imageIndex], 0, nullptr);
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

void VulkanMesh::UpdateUniformBuffers(float dt, uint32_t imageIndex){
	static float time = dt;
	time+=dt;
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), 1024 / (float) 768, 0.1f, 10.0f); // TODO: move to Camera
	ubo.view = ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj[1][1] *= -1; // hack for Vulkan

	void* data;
	vkMapMemory(r_device, m_uniformBuffers[imageIndex].memoryRef, m_uniformBuffers[imageIndex].offset, m_uniformBuffers[imageIndex].size, 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(r_device, m_uniformBuffers[imageIndex].memoryRef);
}

void VulkanMesh::CreateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t imageCount){
	std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
	allocInfo.pSetLayouts = layouts.data();
	
	m_descriptorSets.resize(imageCount);
	VK_CHECK(vkAllocateDescriptorSets(r_device, &allocInfo, m_descriptorSets.data()));
}

void VulkanMesh::UpdateDescriptorSets(){
	for (size_t i = 0; i < m_descriptorSets.size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i].bufferRef;
		bufferInfo.offset = m_uniformBuffers[i].offset;
		bufferInfo.range = m_uniformBuffers[i].size;

		VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = m_descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(r_device, 1, &descriptorWrite, 0, nullptr); // Seems you can'not send array of writes if they all are set to the same bindings
	}
}