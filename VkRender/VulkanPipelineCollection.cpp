#include "VulkanPipelineCollection.h"
#include "render_utils.h"
#include "VulkanSurface.h"
#include "VulkanShaderManager.h"


VulkanPipelineCollection::VulkanPipelineCollection() :
	m_pipelineCache(nullptr)
{
    m_pipelines.resize(2);
}

VulkanPipelineCollection::~VulkanPipelineCollection(){
	vkDestroyRenderPass(r_device, m_pipelines[0].renderPass, 0); // TODO: should be a single render pass
	for (VulkanPipelineSetup &pipelineSetup : m_pipelines){
		vkDestroyPipeline(r_device, pipelineSetup.pipeline, 0);
		vkDestroyPipelineLayout(r_device, pipelineSetup.layout, 0);
		vkDestroyDescriptorSetLayout(r_device, pipelineSetup.descriptorSetLayout, nullptr);
	}
}

void VulkanPipelineCollection::Initialize(VkDevice device, VulkanShaderManager * shaderMgr, VulkanSurface * surface, uint32_t imageCount){
    r_shaderMgr = shaderMgr;
    r_device = device;
    r_surface = surface;

    // TODO: Create pipelines from config file in future
	VkRenderPass renderPass = CreateRenderPass();
	{
		VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout(PipelineType::PT_mesh);
		VkPipelineLayout layout = CreatePipelineLayout(PipelineType::PT_mesh, descriptorSetLayout);
		VkPipeline pipeline = CreateGraphicsPipeline(PipelineType::PT_mesh, layout, renderPass);

		m_pipelines[PipelineType::PT_mesh].layout = layout;
		m_pipelines[PipelineType::PT_mesh].pipeline = pipeline;
		m_pipelines[PipelineType::PT_mesh].renderPass = renderPass;
		m_pipelines[PipelineType::PT_mesh].descriptorSetLayout = descriptorSetLayout;
	}
	{
		VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout(PipelineType::PT_primitive);
		VkPipelineLayout layout = CreatePipelineLayout(PipelineType::PT_primitive, descriptorSetLayout);
		VkPipeline pipeline = CreateGraphicsPipeline(PipelineType::PT_primitive, layout, renderPass);

		m_pipelines[PipelineType::PT_primitive].layout = layout;
		m_pipelines[PipelineType::PT_primitive].pipeline = pipeline;
		m_pipelines[PipelineType::PT_primitive].renderPass = renderPass;
		m_pipelines[PipelineType::PT_primitive].descriptorSetLayout = descriptorSetLayout;
	}
}

const VulkanPipelineCollection::VulkanPipelineSetup& VulkanPipelineCollection::GetPipeline(PipelineType type)const {
    assert(type < PipelineType::PT_size);

    return m_pipelines.at(type);
}

void VulkanPipelineCollection::BeginRenderPass(VkCommandBuffer commandBuffer, PipelineType type, VkFramebuffer framebuffer, uint32_t width, uint32_t height){
	VkClearColorValue color = { 48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1 };
	std::vector<VkClearValue> clearValues(2);
	clearValues[0].color = { color };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo passBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	passBeginInfo.renderPass = m_pipelines[type].renderPass;
	passBeginInfo.framebuffer = framebuffer;
	passBeginInfo.renderArea.extent.width = width;
	passBeginInfo.renderArea.extent.height = height;
	passBeginInfo.clearValueCount = clearValues.size();
	passBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanPipelineCollection::EndRenderPass(VkCommandBuffer commandBuffer){
	vkCmdEndRenderPass(commandBuffer);
}

void VulkanPipelineCollection::BindPipeline(VkCommandBuffer commandBuffer, PipelineType type){
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[type].pipeline);
}

VkPipelineLayout VulkanPipelineCollection::CreatePipelineLayout(PipelineType type, VkDescriptorSetLayout descriptorSetLayout)const {
	assert(type < PipelineType::PT_size);
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(r_device, &pipelineLayoutInfo, 0, &layout));

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
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.depthClampEnable = VK_FALSE;
	createInfo.pRasterizationState = &rasterizationState;

	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &multisampleState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.minDepthBounds = 0.0f; // Optional
	depthStencilState.maxDepthBounds = 1.0f; // Optional
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {}; // Optional
	depthStencilState.back = {}; // Optional
	createInfo.pDepthStencilState = &depthStencilState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.logicOpEnable = VK_FALSE;
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

VkRenderPass VulkanPipelineCollection::CreateRenderPass() const {
    VkFormat format = r_surface->GetSwapchainFormat();

	std::vector<VkAttachmentDescription> attachments(2);
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments[1].format = VK_FORMAT_D32_SFLOAT; // TODO: shortcut. check if it's supported by the device
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthAttachmentRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  };

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachments;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	VkRenderPass renderPass = 0;
	VK_CHECK(vkCreateRenderPass(r_device, &createInfo, 0, &renderPass));

	return renderPass;
}

VkDescriptorSetLayout VulkanPipelineCollection::CreateDescriptorSetLayout(PipelineType type) const{
	assert(type < PipelineType::PT_size);
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	layoutBindings.push_back(uboLayoutBinding);

	if (type != PT_primitive){
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings.push_back(samplerLayoutBinding);
	}

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = layoutBindings.size();
	layoutInfo.pBindings = layoutBindings.data();

	VK_CHECK(vkCreateDescriptorSetLayout(r_device, &layoutInfo, nullptr, &descriptorSetLayout));

	return descriptorSetLayout;
}
