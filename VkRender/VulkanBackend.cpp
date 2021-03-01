#include "VulkanBackend.h"
#include "render_utils.h"

#include <volk.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <vector>

void VulkanBackend::Initialize(){
    // init volk first
    VK_CHECK(volkInitialize());

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