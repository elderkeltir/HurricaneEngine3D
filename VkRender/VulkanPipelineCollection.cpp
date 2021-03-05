#include "VulkanPipelineCollection.h"
#include "render_utils.h"
#include "VulkanSurface.h"
#include "VulkanShaderManager.h"


VulkanPipelineCollection::VulkanPipelineCollection() :
	m_pipelineCache(nullptr)
{
    m_pipelines.resize(PipelineType::PT_size);
}

VulkanPipelineCollection::~VulkanPipelineCollection(){
	for (VulkanPipelineSetup &pipelineSetup : m_pipelines){
		vkDestroyPipeline(r_device, pipelineSetup.pipeline, 0);
		vkDestroyPipelineLayout(r_device, pipelineSetup.layout, 0);
		vkDestroyRenderPass(r_device, pipelineSetup.renderPass, 0);
	}
}

void VulkanPipelineCollection::Initialize(VkDevice device, VulkanShaderManager * shaderMgr, VulkanSurface * surface){
    r_shaderMgr = shaderMgr;
    r_device = device;
    r_surface = surface;

    // Reate pipelines from config file in future
    VkRenderPass renderPass = CreateRenderPass(PipelineType::PT_mesh);
	VkPipelineLayout layout = CreatePipelineLayout(PipelineType::PT_mesh);
    VkPipeline pipeline = CreateGraphicsPipeline(PipelineType::PT_mesh, layout, renderPass);

	m_pipelines[PipelineType::PT_mesh].layout = layout;
    m_pipelines[PipelineType::PT_mesh].pipeline = pipeline;
    m_pipelines[PipelineType::PT_mesh].renderPass = renderPass;
}

const VulkanPipelineCollection::VulkanPipelineSetup& VulkanPipelineCollection::GetPipeline(PipelineType type)const {
    assert(type < PipelineType::PT_size);

    return m_pipelines.at(type);
}

void VulkanPipelineCollection::BeginRenderPass(VkCommandBuffer commandBuffer, PipelineType type, VkFramebuffer framebuffer){
	uint32_t width = 0, height = 0;
	r_surface->GetWindowsExtent(width, height);
	VkClearColorValue color = { 48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1 };
	VkClearValue clearColor = { color };

	VkRenderPassBeginInfo passBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	passBeginInfo.renderPass = m_pipelines[type].renderPass;
	passBeginInfo.framebuffer = framebuffer;
	passBeginInfo.renderArea.extent.width = width;
	passBeginInfo.renderArea.extent.height = height;
	passBeginInfo.clearValueCount = 1;
	passBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanPipelineCollection::EndRenderPass(VkCommandBuffer commandBuffer){
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanPipelineCollection::BindPipeline(VkCommandBuffer commandBuffer, PipelineType type){
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[type].pipeline);
}

VkPipelineLayout VulkanPipelineCollection::CreatePipelineLayout(PipelineType type)const {
	VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(r_device, &createInfo, 0, &layout));

	return layout;
}

VkPipeline VulkanPipelineCollection::CreateGraphicsPipeline(PipelineType type, VkPipelineLayout layout, VkRenderPass renderPass) const {
    assert(type < PipelineType::PT_size);

	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = r_shaderMgr->GetShaderModule(type, VulkanShaderManager::ShaderType::ST_vertex);
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = r_shaderMgr->GetShaderModule(type, VulkanShaderManager::ShaderType::ST_fragment);
	stages[1].pName = "main";

	createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

	VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	createInfo.pVertexInputState = &vertexInput;

	// TODO: temporary, legacy FFP IA
	VkVertexInputBindingDescription stream = { 0, 32, VK_VERTEX_INPUT_RATE_VERTEX };
	VkVertexInputAttributeDescription attrs[3] = {};

	attrs[0].location = 0;
	attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[0].offset = 0;
	attrs[1].location = 1;
	attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attrs[1].offset = 12;
	attrs[2].location = 2;
	attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
	attrs[2].offset = 24;

	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &stream;
	vertexInput.vertexAttributeDescriptionCount = 3;
	vertexInput.pVertexAttributeDescriptions = attrs;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;

	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	createInfo.pRasterizationState = &rasterizationState;

	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &multisampleState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	createInfo.pDepthStencilState = &depthStencilState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = renderPass;

	VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(r_device, m_pipelineCache, 1, &createInfo, 0, &pipeline));

	return pipeline;
}

VkRenderPass VulkanPipelineCollection::CreateRenderPass(PipelineType type) const {
    assert(type < PipelineType::PT_size);

    VkFormat format = r_surface->GetSwapchainFormat();

	VkAttachmentDescription attachments[1] = {};
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachments;

	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
	createInfo.pAttachments = attachments;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;

	VkRenderPass renderPass = 0;
	VK_CHECK(vkCreateRenderPass(r_device, &createInfo, 0, &renderPass));

	return renderPass;
}