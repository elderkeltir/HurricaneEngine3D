#include "VulkanShaderManager.h"
#include "render_utils.h"
#include "VulkanPipelineCollection.h"

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <filesystem>

struct VulkanShaderDB{
    struct VulkanShaderDBSlice{
        struct VulkanShaderDBSlicePerType{
            std::vector<VkShaderModule> m_shadetTypeSlices;
        };
        std::vector<VulkanShaderDBSlicePerType> m_DBslices;
    };

    std::vector<VulkanShaderDBSlice> m_db;
};

typedef VulkanShaderDB::VulkanShaderDBSlice VulkanShaderDBSliceT;
typedef VulkanShaderDB::VulkanShaderDBSlice::VulkanShaderDBSlicePerType VulkanShaderDBSlicePerTypeT;

VulkanShaderManager::VulkanShaderManager(){
	m_shaderDB = new VulkanShaderDB;
    m_shaderDB->m_db.resize(VulkanPipelineCollection::PipelineType::PT_size);
    for (VulkanShaderDBSliceT & slice : m_shaderDB->m_db){
        slice.m_DBslices.resize(ShaderType::ST_size);
    }
}

VulkanShaderManager::~VulkanShaderManager(){

	vkDestroyShaderModule(r_device, m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_mesh).m_DBslices.at(ShaderType::ST_vertex).m_shadetTypeSlices.front(), 0);
	vkDestroyShaderModule(r_device, m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_mesh).m_DBslices.at(ShaderType::ST_fragment).m_shadetTypeSlices.front(), 0);
	vkDestroyShaderModule(r_device, m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_primitive).m_DBslices.at(ShaderType::ST_vertex).m_shadetTypeSlices.front(), 0);
	vkDestroyShaderModule(r_device, m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_primitive).m_DBslices.at(ShaderType::ST_fragment).m_shadetTypeSlices.front(), 0);
	delete m_shaderDB;
	m_shaderDB = nullptr;
}

void VulkanShaderManager::Initialize(VkDevice device, const char * folderPath){
    // TODO: look up through shader folder and load every shader and put into db by name format(e.g. <PipelineType>_<ShaderType><index>.<ext> or w\e)
    r_device = device;
    strcpy(m_folderPath, folderPath);
    std::filesystem::path root_path = std::filesystem::path(m_folderPath);
	{
		std::string vert_shader_path = root_path.string() + "/VkRender/shaders/triangle.vert.spv";
		std::string frag_shader_path = root_path.string() + "/VkRender/shaders/triangle.frag.spv";
		VkShaderModule vs = LoadShader(vert_shader_path.c_str());
		assert(vs);
		VkShaderModule fs = LoadShader(frag_shader_path.c_str());
		assert(fs);

		m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_mesh).m_DBslices.at(ShaderType::ST_vertex).m_shadetTypeSlices.push_back(vs);
		m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_mesh).m_DBslices.at(ShaderType::ST_fragment).m_shadetTypeSlices.push_back(fs);
	}
	{
		std::string vert_shader_path = root_path.string() + "/VkRender/shaders/triangle-primitive.vert.spv";
		std::string frag_shader_path = root_path.string() + "/VkRender/shaders/triangle-primitive.frag.spv";
		VkShaderModule vs = LoadShader(vert_shader_path.c_str());
		assert(vs);
		VkShaderModule fs = LoadShader(frag_shader_path.c_str());
		assert(fs);

		m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_primitive).m_DBslices.at(ShaderType::ST_vertex).m_shadetTypeSlices.push_back(vs);
		m_shaderDB->m_db.at(VulkanPipelineCollection::PipelineType::PT_primitive).m_DBslices.at(ShaderType::ST_fragment).m_shadetTypeSlices.push_back(fs);
	}
}

VkShaderModule VulkanShaderManager::GetShaderModule(VulkanPipelineCollection::PipelineType pipelineType, VulkanShaderManager::ShaderType shader_type, size_t index) const {
    return m_shaderDB->m_db.at(pipelineType).m_DBslices.at(shader_type).m_shadetTypeSlices[index];
}

VkShaderModule VulkanShaderManager::LoadShader(const char* path)
{
	FILE* file = 0;
	file = fopen(path, "rb");
	assert(file);

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	assert(length >= 0);
	fseek(file, 0, SEEK_SET);

	char* buffer = new char[length];
	assert(buffer);

	size_t rc = fread(buffer, 1, length, file);
	assert(rc == size_t(length));
	fclose(file);

	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = length;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

	VkShaderModule shaderModule = 0;
	VK_CHECK(vkCreateShaderModule(r_device, &createInfo, 0, &shaderModule));

	delete[] buffer;

	return shaderModule;
}