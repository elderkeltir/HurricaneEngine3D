#include "VulkanBackend.h"
#include "render_utils.h"
#include "VulkanSurface.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineCollection.h"
#include "VulkanShaderManager.h"
#include "VulkanCommandQueueDispatcher.h"
#include "VulkanMemoryManager.h"
#include "VulkanMesh.h"

#include <volk.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <vector>
#include <cstdio>
#include <cstring>
#include <filesystem>

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
	m_memoryMgr(nullptr)
{
}

VulkanBackend::~VulkanBackend(){
	// TODO: replace allocations with intrusive ptr when guys will implement it
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
	delete m_cmdQueueDispatcher;
	m_cmdQueueDispatcher = nullptr;

	vkDestroyDevice(m_device, 0);
	vkDestroyInstance(m_instance, 0);
}

void VulkanBackend::Initialize(const char * rootFolder){
	strcpy(m_rootFolder, rootFolder);

	// Create Surface
	m_surface = new VulkanSurface;

    // init volk
    VK_CHECK(volkInitialize());

	// create Instance
	CreateInstance();
	assert(m_instance);
	volkLoadInstance(m_instance);

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
	m_pipelineCollection->Initialize(m_device, m_shaderMgr, m_surface);

	// Create SwapChain
	uint32_t windowWidth = 0, windowHeight = 0;
	m_surface->GetWindowsExtent(windowWidth, windowHeight);
	assert(!!windowWidth && !!windowHeight);
	m_swapChain = new VulkanSwapChain(m_physicalDevice, m_device, m_surface, m_cmdQueueDispatcher->GetQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics).familyQueueIndex, m_surface->GetSwapchainFormat(), windowWidth, windowHeight, m_pipelineCollection->GetPipeline(VulkanPipelineCollection::PipelineType::PT_mesh).renderPass, m_bufferSize);

	m_memoryMgr = new VulkanMemoryManager;
	m_memoryMgr->Initialize(m_physicalDevice, m_device);
	// TODO: render scene? how to store meshes(vertex + index + UBO + texture) in render backend?
	std::filesystem::path root_path = std::filesystem::path(m_rootFolder);
	std::string obj_path = root_path.string() + "/extern/meshoptimizer/demo/pirate.obj";
	VulkanMesh mesh;
	mesh.Initialize(obj_path.c_str(), m_memoryMgr, m_device);
}

void VulkanBackend::Render(){
	if (m_surface->PollWindowEvents())
	{
		m_swapChain->ResizeOnNeed();

		uint32_t width = 0u, height = 0u;
		m_surface->GetWindowsExtent(width, height);
		uint32_t nextImg_idx = m_swapChain->AcquireNextImage(m_cmdQueueDispatcher->GetAquireSemaphore());
		VkCommandBuffer commandBuffer = m_cmdQueueDispatcher->GetCommandBuffer(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);
		//m_cmdQueueDispatcher->ResetCommandPool(VulkanCommandQueueDispatcher::QueueType::QT_graphics);
		m_cmdQueueDispatcher->BeginCommandBuffer(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);
		m_swapChain->BindRenderStartBarrier(commandBuffer, nextImg_idx);
		m_pipelineCollection->BeginRenderPass(commandBuffer, VulkanPipelineCollection::PipelineType::PT_mesh, m_swapChain->GetFB(nextImg_idx));

		// TODO: move this somewhere later
		VkViewport viewport = { 0, float(height), float(width), -float(height), 0, 1 };
		VkRect2D scissor = { {0, 0}, {uint32_t(width), uint32_t(height)} };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		m_pipelineCollection->BindPipeline(commandBuffer, VulkanPipelineCollection::PipelineType::PT_mesh);

		for(VulkanMesh &mesh : m_meshes){
			mesh.Render(commandBuffer);
		}

		m_pipelineCollection->EndRenderPass(commandBuffer);
		m_swapChain->BindRenderEndBarrier(commandBuffer, nextImg_idx);

		m_cmdQueueDispatcher->SubmitQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx);
		m_cmdQueueDispatcher->PresentQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics, nextImg_idx, m_swapChain);
	}
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

	for (uint32_t i = 0; i < physicalDevices.size(); ++i)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

		printf("GPU%d: %s\n", i, props.deviceName);

		uint32_t familyIndex = VulkanCommandQueueDispatcher::TestFamilQueueyIndex(VulkanCommandQueueDispatcher::QueueType::QT_graphics, physicalDevices[i]);
		if (familyIndex == VK_QUEUE_FAMILY_IGNORED)
			continue;

		auto res = glfwGetPhysicalDevicePresentationSupport(m_instance, physicalDevices[i], familyIndex);
		if (!res)
			continue;

		if (!discrete && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			discrete = physicalDevices[i];
		}

		if (!fallback)
		{
			fallback = physicalDevices[i];
		}
	}

	VkPhysicalDevice result = discrete ? discrete : fallback;

	if (result)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(result, &props);

		printf("Selected GPU %s\n", props.deviceName);
	}
	else
	{
		printf("ERROR: No GPUs found\n");
	}

	return result;
}

void VulkanBackend::CreateDevice(){
	float queuePriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = m_cmdQueueDispatcher->GetQueue(VulkanCommandQueueDispatcher::QueueType::QT_graphics).familyQueueIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	#ifdef VK_USE_PLATFORM_METAL_EXT
		VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
	#endif
		//VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, 1:35
	};

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueInfo;

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, 0, &m_device));
}