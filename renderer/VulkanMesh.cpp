#include "VulkanMesh.h"
#include "VulkanMemoryManager.h"
#include "VulkanCommandQueueDispatcher.h"
#include "VulkanPipelineCollection.h"
#include "VulkanBackend.h"
#include "VulkanCamera.h"

#include <meshoptimizer.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // TODO: move loading image to another class. probably after vfs implementation?
#pragma warning(disable : 4996)
#define FAST_OBJ_IMPLEMENTATION
#include <../extern/fast_obj.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <string>

#include <glm/gtx/quaternion.hpp>

VulkanMesh::VulkanMesh(VulkanBackend * backend) : 
	r_device(nullptr),
	m_pipelineType(iface::RenderPipelineCollection::PipelineType::PT_size),
	r_backend(backend)
{
	m_color[0] = 0.7f;
	m_color[1] = 0.5f;
	m_color[2] = 0.2f;
	m_color[3] = 1.f;
}

VulkanMesh::~VulkanMesh(){
	// TODO: move this to memory manager
	if (r_device){
		if (m_imagePtr.sampler)
			vkDestroySampler(r_device, m_imagePtr.sampler, nullptr);
		if (m_imagePtr.imageView)
			vkDestroyImageView(r_device, m_imagePtr.imageView, nullptr);
		if (m_imagePtr.imageRef)
			vkDestroyImage(r_device, m_imagePtr.imageRef, nullptr);
		if (m_imagePtr.memoryRef)
			vkFreeMemory(r_device, m_imagePtr.memoryRef, nullptr);
	}
}

VulkanMesh::VulkanMesh(VulkanMesh && other){
	this->m_vertices.swap(other.m_vertices);
	this->m_indices.swap(other.m_indices);
	std::swap(this->m_vBuffPtr, other.m_vBuffPtr);
	std::swap(this->m_iBuffPtr, other.m_iBuffPtr);
	std::swap(this->m_vsBuffPtr, other.m_vsBuffPtr);
	std::swap(this->m_tsBuffPtr, other.m_tsBuffPtr);
	std::swap(this->m_imagePtr, other.m_imagePtr);
	this->m_uniformBuffers.swap(other.m_uniformBuffers);
	this->m_descriptorSets.swap(other.m_descriptorSets);
	std::swap(this->r_device, other.r_device);
	std::swap(this->m_pipelineType, other.m_pipelineType);
	std::swap(this->r_pipelineCollection, other.r_pipelineCollection);
	std::swap(this->m_color, other.m_color);
	std::swap(this->r_backend, other.r_backend);
	std::swap(this->m_model, other.m_model);
}

VulkanMesh& VulkanMesh::operator=(VulkanMesh&& other){
	this->m_vertices.swap(other.m_vertices);
	this->m_indices.swap(other.m_indices);
	std::swap(this->m_vBuffPtr, other.m_vBuffPtr);
	std::swap(this->m_iBuffPtr, other.m_iBuffPtr);
	std::swap(this->m_vsBuffPtr, other.m_vsBuffPtr);
	std::swap(this->m_tsBuffPtr, other.m_tsBuffPtr);
	std::swap(this->m_imagePtr, other.m_imagePtr);
	this->m_uniformBuffers.swap(other.m_uniformBuffers);
	this->m_descriptorSets.swap(other.m_descriptorSets);
	std::swap(this->r_device, other.r_device);
	std::swap(this->m_pipelineType, other.m_pipelineType);
	std::swap(this->r_pipelineCollection, other.r_pipelineCollection);
	std::swap(this->m_color, other.m_color);
	std::swap(this->r_backend, other.r_backend);
	std::swap(this->m_model, other.m_model);

	return *this;
}

// TODO:
void VulkanMesh::UpdateModelMx(float * mx){
	glm::vec3 p(mx[4], mx[5], mx[6]);
	glm::quat q(mx[0], mx[1], mx[2], mx[3]);
	glm::mat4 model(1.0f);
	model = glm::translate(model, p);
	glm::mat4 RotationMatrix = glm::toMat4(q);
	m_model = model * RotationMatrix;
	m_model = glm::scale(m_model, glm::vec3(mx[7], mx[8], mx[9]));
}

void VulkanMesh::Initialize(const char *path,
                    const char *texturePath, 
                    VulkanMemoryManager * memoryMgr, 
                    VulkanCommandQueueDispatcher * queueDispatcher, 
                    VkDevice device, VkDescriptorPool descriptorPool, 
                    iface::RenderPipelineCollection::PipelineType pipelineType, 
                    VulkanPipelineCollection *pipelineCollection, 
                    uint32_t imageCount){
    assert(ParseObj(path));
    r_device = device;
	r_pipelineCollection = pipelineCollection;
	m_pipelineType = pipelineType;
    
	// Allocate buffers for vertex, index buffers
    assert(memoryMgr);
    m_vBuffPtr = memoryMgr->AllocateBuffer(m_vertices.size() * sizeof(Vertex), VulkanMemoryManager::BufferUsageType::BUT_vertex_buffer);
    m_vBuffPtr.Validate();

    m_iBuffPtr = memoryMgr->AllocateBuffer(m_indices.size() * sizeof(uint32_t), VulkanMemoryManager::BufferUsageType::BUT_index_buffer);
    m_iBuffPtr.Validate();

	m_vsBuffPtr = memoryMgr->AllocateBuffer(m_vertices.size() * sizeof(Vertex), VulkanMemoryManager::BufferUsageType::BUT_transfer_src);
    m_vsBuffPtr.Validate();

	// load vertex, index data to memory
    void* vData = 0;
	VK_CHECK(vkMapMemory(r_device, m_vsBuffPtr.memoryRef, m_vsBuffPtr.offset, m_vsBuffPtr.size, 0, &vData));
    assert(vData);
    memcpy(vData, m_vertices.data(), m_vertices.size() * sizeof(Vertex)); // TODO: for primitive we don't actually need uv coords
    vkUnmapMemory(r_device, m_vsBuffPtr.memoryRef);

	// copy from staging buffer to vertex
	queueDispatcher->CopyBuffer(m_vsBuffPtr, m_vBuffPtr);

    void* iData = 0;
	VK_CHECK(vkMapMemory(r_device, m_iBuffPtr.memoryRef, m_iBuffPtr.offset, m_iBuffPtr.size, 0, &iData));
    assert(iData);
    memcpy(iData, m_indices.data(), m_indices.size() * sizeof(uint32_t));
    vkUnmapMemory(r_device, m_iBuffPtr.memoryRef);

	// texture
	if (m_pipelineType != iface::RenderPipelineCollection::PipelineType::PT_primitive){
		LoadTexture(memoryMgr, queueDispatcher, texturePath);
	}

	// allocate uniform buffers
	m_uniformBuffers.resize(imageCount);
	for (uint32_t i = 0u; i < imageCount; i++){
		m_uniformBuffers[i] = memoryMgr->AllocateBuffer(sizeof(UniformBufferObject), VulkanMemoryManager::BufferUsageType::BUT_uniform_buffer);
    	m_uniformBuffers[i].Validate();
	}

	const VulkanPipelineCollection::VulkanPipelineSetup &pipeline = r_pipelineCollection->GetPipeline(m_pipelineType);

	CreateDescriptorSets(descriptorPool, pipeline.descriptorSetLayout, imageCount);
	UpdateDescriptorSets(); // TODO: maybe there is some reasons to add flexibility here?
}

void VulkanMesh::Render(float dt, VkCommandBuffer commandBuffer, uint32_t imageIndex){
	UpdateUniformBuffers(dt, imageIndex);
	
	const VulkanPipelineCollection::VulkanPipelineSetup &pipeline = r_pipelineCollection->GetPipeline(m_pipelineType);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vBuffPtr.bufferRef, &m_vBuffPtr.offset);
	vkCmdBindIndexBuffer(commandBuffer, m_iBuffPtr.bufferRef, m_iBuffPtr.offset, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &m_descriptorSets[imageIndex], 0, nullptr);
	for (const VkPushConstantRange &pushConstant : pipeline.pushConstants){
		vkCmdPushConstants(commandBuffer, pipeline.layout, pushConstant.stageFlags, pushConstant.offset, pushConstant.size, m_color); // TODO: what if multiple push constants? think a bit later
	}
	
	vkCmdDrawIndexed(commandBuffer, m_indices.size(), 1, 0, 0, 0);
}

iface::RenderPipelineCollection::PipelineType VulkanMesh::GetPipelineType() const{
	return m_pipelineType;
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
				1.f - obj->texcoords[gi.t * 2 + 1], // TODO: god damn obj rotates y!
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
	ubo.model = m_model;
	auto pos = glm::vec3(m_model[3]);
	ubo.proj = r_backend->GetCamera()->GetProjection(); // TODO: move away to scene UBO when scene is implemented
	ubo.view = r_backend->GetCamera()->GetView();

	//ubo.proj = glm::perspective(glm::radians(45.0f), 1280 / (float) 720, 0.1f, 10.0f); // TODO: move to Camera
	//ubo.view = glm::lookAt(glm::vec3(5.0f, 5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

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

		std::vector<VkWriteDescriptorSet> descriptorWrites(1);
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		if (m_pipelineType != iface::RenderPipelineCollection::PipelineType::PT_primitive){
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_imagePtr.imageView;
			imageInfo.sampler = m_imagePtr.sampler;
			
			descriptorWrites.resize(2);
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;
		}

		vkUpdateDescriptorSets(r_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr); // Seems you can'not send array of writes if they all are set to the same bindings
	}
}

void VulkanMesh::LoadTexture(VulkanMemoryManager * memoryMgr, VulkanCommandQueueDispatcher * queueDispatcher, const char *texturePath){
	int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
	assert(pixels);

	m_tsBuffPtr = memoryMgr->AllocateBuffer(imageSize, VulkanMemoryManager::BufferUsageType::BUT_transfer_src);
    m_tsBuffPtr.Validate();

	void* data;
	vkMapMemory(r_device, m_tsBuffPtr.memoryRef, m_tsBuffPtr.offset, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(r_device, m_tsBuffPtr.memoryRef);

	stbi_image_free(pixels);

	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

	m_imagePtr = memoryMgr->AllocateImage(texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, VulkanMemoryManager::BufferUsageType::BUT_transfer_dst | VulkanMemoryManager::BufferUsageType::BUT_sampled, VK_IMAGE_ASPECT_COLOR_BIT, true);

	queueDispatcher->TransitionImageLayout(m_imagePtr.imageRef, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	queueDispatcher->CopyBufferToImage(m_tsBuffPtr, m_imagePtr.imageRef, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	queueDispatcher->TransitionImageLayout(m_imagePtr.imageRef, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}	