#include "VulkanBackend.h"
#include "render_utils.h"
#include "VulkanSurface.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineCollection.h"
#include "VulkanShaderManager.h"
#include "VulkanCommandQueueDispatcher.h"
#include "VulkanMemoryManager.h"
#include "VulkanMesh.h"
#include "VulkanDescriptorSetOrginizer.h"
#include "VulkanCamera.h"

#include <volk.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <vector>
#include <cstdio>
#include <cstring>
#include <filesystem>

#ifdef WIN32
#include <Windows.h>
#endif

void RenderObject::SetMesh(VulkanMesh * mesh)
{
	m_mesh = mesh;
}

void RenderObject::Update(float *mx){
	m_mesh->UpdateModelMx(mx);
}

static uint64_t frame = 0;

#ifdef _DEBUG
VkDebugReportCallbackEXT gDdebugCallback = nullptr;

VkBool32 debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	const char* type =
		(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		? "ERROR"
		: (flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
		? "WARNING"
		: "INFO";

	char message[4096];
	snprintf(message, (sizeof(message) / sizeof((message)[0])), "%s: %s\n", type, pMessage);

	printf("%s", message);

#ifdef _WIN32
	OutputDebugStringA(message);
#endif

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		assert(!"Validation error encountered!");

	return VK_FALSE;
}

VkDebugReportCallbackEXT registerDebugCallback(VkInstance instance)
{
	VkDebugReportCallbackCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
	createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	createInfo.pfnCallback = debugReportCallback;

	VkDebugReportCallbackEXT callback = 0;
	VK_CHECK(vkCreateDebugReportCallbackEXT(instance, &createInfo, 0, &callback));

	return callback;
}
#endif // _DEBUG

VulkanBackend::VulkanBackend():
	m_bufferSize(3u),
	m_instance(nullptr),
	m_physicalDevice(nullptr),
	m_device(nullptr),
	m_swapChain(nullptr),
	m_surface(nullptr),
	m_pipelineCollection(nullptr),
	m_shaderMgr(nullptr),
	m_cmdQueueDispatcher(nullptr),
	m_memoryMgr(nullptr),
	m_descriptorSetOrganizer(nullptr)
{
}

VulkanBackend::~VulkanBackend(){
	// TODO: replace allocations with intrusive ptr when guys will implement it
	delete m_cmdQueueDispatcher;
	m_cmdQueueDispatcher = nullptr;
	m_meshes.clear(); // TODO: may remove after fix for mesh destructor
	delete m_descriptorSetOrganizer;
	m_descriptorSetOrganizer = nullptr;
	delete m_memoryMgr;
	m_memoryMgr = nullptr;
	delete m_pipelineCollection;
	m_pipelineCollection = nullptr;
	delete m_swapChain;
	m_swapChain = nullptr;
	delete m_shaderMgr;
	m_shaderMgr = nullptr;
	delete m_surface;
	m_surface = nullptr;
#ifdef _DEBUG
	vkDestroyDebugReportCallbackEXT(m_instance, gDdebugCallback, 0);
#endif

	vkDestroyDevice(m_device, 0);
	vkDestroyInstance(m_instance, 0);
}

void VulkanBackend::Initialize(const char * rootFolder){
	strcpy(m_rootFolder, rootFolder);

	// Create Surface
	m_surface = new VulkanSurface(this);

    // init volk
    VK_CHECK(volkInitialize());

	// create Instance
	CreateInstance();
	assert(m_instance);
	volkLoadInstance(m_instance);

#ifdef _DEBUG
	gDdebugCallback = registerDebugCallback(m_instance);
#endif // _DEBUG

	// pick Physical device
	uint32_t physicalDeviceCount;
	VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr));
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()));
	m_physicalDevice = PickPhysicalDevice(physicalDevices);
	assert(m_physicalDevice);

	// first cmdqueue init pass
	m_cmdQueueDispatcher = new VulkanCommandQueueDispatcher(m_physicalDevice);

	// create Device
	CreateDevice();
	assert(m_device);
	volkLoadDevice(m_device);

	// second cmdqueue init pass
	m_cmdQueueDispatcher->Initialize(m_device, m_bufferSize);
	m_surface->Initialize(m_instance, m_physicalDevice);

	// Check phys device + queue + surface combo
	VkBool32 presentSupported = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_cmdQueueDispatcher->GetQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics).familyQueueIndex, m_surface->Vk_surface(), &presentSupported));
	assert(presentSupported);

	// Init Shader DB and manager
	m_shaderMgr = new VulkanShaderManager;
	m_shaderMgr->Initialize(m_device, m_rootFolder); // TODO: shaders folder in future

	// Create Graphic Pipeline
	m_pipelineCollection = new VulkanPipelineCollection;
	m_pipelineCollection->Initialize(m_device, m_shaderMgr, m_surface, m_bufferSize);

	// memory mgr
	m_memoryMgr = new VulkanMemoryManager;
	m_memoryMgr->Initialize(m_physicalDevice, m_device);

	// Create SwapChain
	uint32_t windowWidth = 0, windowHeight = 0;
	m_surface->GetWindowsExtent(windowWidth, windowHeight);
	assert(!!windowWidth && !!windowHeight);
	m_swapChain = new VulkanSwapChain(m_physicalDevice, m_device, m_surface, m_memoryMgr, m_cmdQueueDispatcher->GetQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics).familyQueueIndex, m_surface->GetSwapchainFormat(), windowWidth, windowHeight, m_pipelineCollection->GetRenderPass(), m_bufferSize);
	m_swapChain->InitializeSwapChain();

	// Descriptor sets
	m_descriptorSetOrganizer = new VulkanDescriptorSetOrginizer;
	m_descriptorSetOrganizer->Initialize(m_device);

	// Camera
	m_camera = new VulkanCamera(this);
	glm::vec3 pos(5.0f, 12.0f, 38.0f);
	glm::vec3 dir(-5.0f, -5.0f, 5.0f);
	m_camera->Initialize(windowWidth, windowHeight, pos, dir);

	// TODO: render scene? how to store meshes(vertex + index + UBO + texture) in render backend?
	std::filesystem::path root_path = std::filesystem::path(m_rootFolder);

	// TODO: as is unless we implement vfs
#ifdef _WIN32
	std::string obj_path = root_path.string() + "\\content\\Madara_Uchiha\\mesh\\Madara_Uchiha.obj";
	std::string texturePath = root_path.string() + "\\content\\Madara_Uchiha\\textures\\_Madara_texture_main_mAIN.png";
#else
	std::string obj_path = root_path.string() + "/content/Madara_Uchiha/mesh/Madara_Uchiha.obj";
	std::string texturePath = root_path.string() + "/content/Madara_Uchiha/textures/_Madara_texture_main_mAIN.png";
#endif //_WIN32

	m_meshes.reserve(32);
	// {
	// 	VulkanMesh mesh(this);
	// 	mesh.Initialize(obj_path.c_str(), texturePath.c_str(), m_memoryMgr, m_cmdQueueDispatcher, m_device, m_descriptorSetOrganizer->GetDescriptorPool(), VulkanPipelineCollection::PipelineType::PT_mesh, m_pipelineCollection, m_bufferSize);
	// 	m_meshes.push_back(std::move(mesh));
	// }
	// {
	// 	VulkanMesh mesh(this);
	// 	obj_path = root_path.string() + "/content/Primitives/capsule.obj";
	// 	mesh.Initialize(obj_path.c_str(), texturePath.c_str(), m_memoryMgr, m_cmdQueueDispatcher, m_device, m_descriptorSetOrganizer->GetDescriptorPool(), VulkanPipelineCollection::PipelineType::PT_primitive, m_pipelineCollection, m_bufferSize);
	// 	m_meshes.push_back(std::move(mesh));
	// }
}

void VulkanBackend::Render(float dt){
	if (m_surface->PollWindowEvents())
	{
		uint32_t width = 0u, height = 0u;
		m_swapChain->ResizeOnNeed(width, height);

		uint32_t nextImg_idx = m_swapChain->AcquireNextImage(m_cmdQueueDispatcher->GetAquireSemaphore());
		VkCommandBuffer commandBuffer = m_cmdQueueDispatcher->GetCommandBuffer(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);
		m_cmdQueueDispatcher->BeginCommandBuffer(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);
		//m_swapChain->BindRenderStartBarrier(commandBuffer, nextImg_idx); // TODO: clean up? we can do transactions using render pass dependency. which one is better?
		m_pipelineCollection->BeginRenderPass(commandBuffer, m_swapChain->GetFB(nextImg_idx), width, height);

		// TODO: move this somewhere later
		VkViewport viewport = { 0, float(height), float(width), -float(height), 0, 1 };
		VkRect2D scissor = { {0, 0}, {uint32_t(width), uint32_t(height)} };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Update camera
		m_camera->Update(dt);

		for(VulkanMesh &mesh : m_meshes){
			VulkanPipelineCollection::PipelineType pipelineType = mesh.GetPipelineType();
			m_pipelineCollection->BindPipeline(commandBuffer, pipelineType); // TODO: sort by pipeline somewhere. to avoid unnecessary pipeline switches
			mesh.Render(dt, commandBuffer, nextImg_idx);
		}

		m_pipelineCollection->EndRenderPass(commandBuffer);
		//m_swapChain->BindRenderEndBarrier(commandBuffer, nextImg_idx);
		m_cmdQueueDispatcher->EndCommandBuffer(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);

		m_cmdQueueDispatcher->SubmitQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);
		m_cmdQueueDispatcher->PresentQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx, m_swapChain);

		frame++;
	}
}

bool VulkanBackend::IsRunning(){
	return m_surface->PollWindowEvents();
}

RenderObject * VulkanBackend::CreateObject(float* mx, bool texturedMesh){
	VulkanMesh mesh(this);
	if (texturedMesh){
		std::filesystem::path root_path = std::filesystem::path(m_rootFolder);
		std::string obj_path = root_path.string() + "\\content\\Madara_Uchiha\\mesh\\Madara_Uchiha.obj";
		std::string texturePath = root_path.string() + "\\content\\Madara_Uchiha\\textures\\_Madara_texture_main_mAIN.png";
		mesh.Initialize(obj_path.c_str(), texturePath.c_str(), m_memoryMgr, m_cmdQueueDispatcher, m_device, m_descriptorSetOrganizer->GetDescriptorPool(), VulkanPipelineCollection::PipelineType::PT_mesh, m_pipelineCollection, m_bufferSize);
		mesh.UpdateModelMx(mx);
		m_meshes.push_back(std::move(mesh));
	}
	else{
		std::filesystem::path root_path = std::filesystem::path(m_rootFolder);
		std::string obj_path = root_path.string() + "/content/Primitives/box.obj";
		mesh.Initialize(obj_path.c_str(), "", m_memoryMgr, m_cmdQueueDispatcher, m_device, m_descriptorSetOrganizer->GetDescriptorPool(), VulkanPipelineCollection::PipelineType::PT_primitive, m_pipelineCollection, m_bufferSize);
		mesh.UpdateModelMx(mx);
		m_meshes.push_back(std::move(mesh));
	}

	RenderObject *obj = new RenderObject;
	VulkanMesh * meshP = &(m_meshes.back()); // TODO: very bad, vector can reallocate it's buffer
	obj->SetMesh(meshP);

	return obj;
}

VulkanCamera * VulkanBackend::GetCamera(){
	return m_camera;
}

const char *VulkanBackend::GetRootPath() const{
	return m_rootFolder;
}

void VulkanBackend::CreateInstance(){
	const uint32_t requiredApiVersion = VK_API_VERSION_1_1;
	const uint32_t recommendedApiVersion = VK_API_VERSION_1_2;
	uint32_t supportedApiVersion = 0;
	VK_CHECK(vkEnumerateInstanceVersion(&supportedApiVersion));

	const uint32_t apiVersion = (supportedApiVersion >= recommendedApiVersion) ? recommendedApiVersion : ((supportedApiVersion >= requiredApiVersion)  ? requiredApiVersion : 0u);

	if (apiVersion == 0u){
		assert(apiVersion);
	}

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = apiVersion;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG
	const char* debugLayers[] =
	{
		"VK_LAYER_KHRONOS_validation",
	};

	createInfo.ppEnabledLayerNames = debugLayers;
	createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif
	std::vector<const char *> surfaceExt = VulkanSurface::GetRequiredExtension();
	std::vector<const char *> extensions;
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	extensions.insert(extensions.end(), surfaceExt.begin(), surfaceExt.end());

	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = extensions.size();

	VK_CHECK(vkCreateInstance(&createInfo, 0, &m_instance));
}

VkPhysicalDevice VulkanBackend::PickPhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices) const {
	VkPhysicalDevice discrete = 0;
	VkPhysicalDevice fallback = 0;

	for (uint32_t i = 0; i < physicalDevices.size(); ++i) {
		bool graphicsSupported = false;
		bool transferSupported = false;
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

		// TODO: save somewhere
		VkDeviceSize alignment = props.limits.minUniformBufferOffsetAlignment;

		printf("GPU%d: %s\n", i, props.deviceName); // TODO: some ifdef for linux and perf build to select proprietary amdgpu driver to actually use amd perf tool
		{
			uint32_t familyIndex = VulkanCommandQueueDispatcher::TestFamilQueueyIndex(physicalDevices[i], VK_QUEUE_GRAPHICS_BIT);
			if (familyIndex != VK_QUEUE_FAMILY_IGNORED)
			{
				if (glfwGetPhysicalDevicePresentationSupport(m_instance, physicalDevices[i], familyIndex))
				{
					graphicsSupported = true;
				}
			}
		}
		{
			uint32_t familyIndex = VulkanCommandQueueDispatcher::TestFamilQueueyIndex(physicalDevices[i], VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT);
			if (familyIndex != VK_QUEUE_FAMILY_IGNORED)
			{
				if (glfwGetPhysicalDevicePresentationSupport(m_instance, physicalDevices[i], familyIndex))
				{
					transferSupported = true;
				}
			}
		}

		if (!discrete && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && graphicsSupported && transferSupported) {
			discrete = physicalDevices[i];
		}

		if (!fallback && graphicsSupported) {
			fallback = physicalDevices[i];
		}
	}

	VkPhysicalDevice result = discrete ? discrete : fallback;

	if (result) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(result, &props);

		printf("Selected GPU %s\n", props.deviceName);
	}
	else {
		printf("ERROR: No GPUs found\n");
	}

	return result;
}

void VulkanBackend::CreateDevice(){
	float queuePriorities[] = { 1.0f };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(2);

	queueCreateInfos[0] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfos[0].queueFamilyIndex = m_cmdQueueDispatcher->GetQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics).familyQueueIndex;
	queueCreateInfos[0].queueCount = 1;
	queueCreateInfos[0].pQueuePriorities = queuePriorities;

	queueCreateInfos[1] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfos[1].queueFamilyIndex = m_cmdQueueDispatcher->GetQueue(VulkanCommandQueueDispatcher::QueueType::QT_transfer).familyQueueIndex;
	queueCreateInfos[1].queueCount = 1;
	queueCreateInfos[1].pQueuePriorities = queuePriorities;

	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	#ifdef VK_USE_PLATFORM_METAL_EXT
		VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
	#endif
		//VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, 1:35
	};

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	// TODO: shortcut. we need to check is it supported vkGetPhysicalDeviceFeatures
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	createInfo.pEnabledFeatures = &deviceFeatures;

	VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, 0, &m_device));
}