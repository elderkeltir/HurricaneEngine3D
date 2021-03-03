#include "VulkanBackend.h"
#include "render_utils.h"
#include "VulkanSurface.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineCollection.h"
#include "VulkanShaderManager.h"
#include "VulkanCommandQueueDispatcher.h"

#include <volk.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <vector>
#include <cstdio>
#include <cstring>

VulkanBackend::VulkanBackend():
	m_device(nullptr),
{
}

VulkanBackend::~VulkanBackend(){
	delete m_cmdQueueDispatcher; // TODO: replace allocations with intrusive ptr when guys will implement it
	m_cmdQueueDispatcher = nullptr;
}

void VulkanBackend::Initialize(const char * rootFolder){
	strcpy(m_rootFolder, rootFolder);
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
	m_cmdQueueDispatcher->Initialize(m_device);

	// Create Surface
	m_surface = new VulkanSurface;
	m_surface->Initialize(m_instance, m_physicalDevice);

	// Check phys device + queue + surface combo
	VkBool32 presentSupported = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_cmdQueueDispatcher->GetFamilyQueueIndex(VulkanCommandQueueDispatcher::QueueType::QT_graphics), m_surface->Vk_surface(), &presentSupported));
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
	m_swapChain = new VulkanSwapChain(m_physicalDevice, m_device, m_surface, m_cmdQueueDispatcher->GetFamilyQueueIndex(VulkanCommandQueueDispatcher::QueueType::QT_graphics), m_surface->GetSwapchainFormat(), windowWidth, windowHeight, m_pipelineCollection->GetPipeline(VulkanPipelineCollection::PipelineType::PT_mesh).renderPass);

	
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

	uint32_t glfwExtensionsNum = 0;
	const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsNum);
	std::vector<const char*> extensions;
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	for (size_t i = 0; i < glfwExtensionsNum; i++){
		extensions.push_back(glfwExtensions[i]);
	}

	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = extensions.size();

	VK_CHECK(vkCreateInstance(&createInfo, 0, &m_instance));
}

VkPhysicalDevice VulkanBackend::PickPhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices) const {
	auto supportsPresentation = [](VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t familyIndex){
		return glfwGetPhysicalDevicePresentationSupport(instance, physicalDevice, familyIndex);
	};

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

		if (!supportsPresentation(m_instance, physicalDevices[i], familyIndex))
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
	queueInfo.queueFamilyIndex = m_cmdQueueDispatcher->GetFamilyQueueIndex(VulkanCommandQueueDispatcher::QueueType::QT_graphics);
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