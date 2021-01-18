#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <cassert>

#pragma warning( disable : 26812 )

#define VK_CHECK(call) \
	do { \
		VkResult result = call; \
		assert(result == VK_SUCCESS); \
	} while(0)

VkInstance createInstance() {
	VkApplicationInfo appinfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appinfo.apiVersion = VK_API_VERSION_1_1; // TODO: check supported versions
	VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appinfo;

#ifdef _DEBUG
	const char* debugLayers[] =
	{
		"VK_LAYER_KHRONOS_validation",
	};

	createInfo.ppEnabledLayerNames = debugLayers;
	createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif

	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	};

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

	return instance;
}

VkPhysicalDevice pickPhysDevice(VkPhysicalDevice * devices, uint32_t num) {

	VkPhysicalDeviceProperties physicalDevProps;
	for (uint32_t i = 0; i < num; i++) {
		vkGetPhysicalDeviceProperties(devices[i], &physicalDevProps);

		if (physicalDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return devices[i];
		}
	}
	assert(false);
	return devices[0];
}

VkDevice createDevice(VkPhysicalDevice physDevice, uint32_t * familyIdx) {
	*familyIdx = 0; // TODO comupe from queue properties
	float queueProperties[] = { 0.f };
	VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfo.queueFamilyIndex = *familyIdx;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queueProperties;

	VkDevice device = 0;
	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	deviceCreateInfo.ppEnabledExtensionNames = extensions;
	deviceCreateInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VK_CHECK(vkCreateDevice(physDevice, &deviceCreateInfo, 0, &device));

	return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkSurfaceKHR surface = 0;
	VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hinstance = GetModuleHandle(0);
	createInfo.hwnd = glfwGetWin32Window(window);
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface));

	return surface;
#else
#error Unsupported Platform
#endif
}

int main() {
	// Window
	assert(glfwInit());
	GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);

	// Vulkan
	VkInstance instance = createInstance();
	assert(instance);
	
	VkPhysicalDevice physicalDevices[16];
	uint32_t physDevNum = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physDevNum, physicalDevices));

	VkPhysicalDevice physDevice = pickPhysDevice(physicalDevices, physDevNum);

	uint32_t familyIdx = 0;
	VkDevice device = createDevice(physDevice, &familyIdx);

	VkSurfaceKHR surface = createSurface(instance, window);

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	VkSwapchainKHR swapchain = 0;
	VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));

	// GLFW

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	vkDestroySurfaceKHR(instance, surface, 0);
	vkDestroyDevice(device, 0);
	vkDestroyInstance(instance, 0);
	

	return 0;
}