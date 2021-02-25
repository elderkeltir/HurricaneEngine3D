// #include <volk.h>
// #include <GLFW/glfw3.h>
// #include <assert.h>
// #include <cstdio>

 #include "Render.h"

// #define VK_CHECK(call) \
// 	do { \
// 		VkResult result_ = call; \
// 		assert(result_ == VK_SUCCESS); \
// 	} while (0)

// #ifndef ARRAYSIZE
// #define ARRAYSIZE(array) (sizeof(array) / sizeof((array)[0]))
// #endif

int main(){
#ifdef VK_USE_PLATFORM_WIN32_KHR
	main_render("../extern/meshoptimizer/demo/pirate.obj");
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    main_render("extern/meshoptimizer/demo/pirate.obj");
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    main_render("../../extern/meshoptimizer/demo/pirate.obj");
#endif
}

// VkInstance createInstance()
// {
// 	// SHORTCUT: In real Vulkan applications you should probably check if 1.1 is available via vkEnumerateInstanceVersion
// 	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
// 	appInfo.apiVersion = VK_API_VERSION_1_1;

// 	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
// 	createInfo.pApplicationInfo = &appInfo;

// #ifdef _DEBUG
// 	const char* debugLayers[] =
// 	{
// 		"VK_LAYER_KHRONOS_validation",
// 	};

// 	createInfo.ppEnabledLayerNames = debugLayers;
// 	createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
// #endif

// 	const char* extensions[] =
// 	{
// 		VK_KHR_SURFACE_EXTENSION_NAME,
// #ifdef VK_USE_PLATFORM_WIN32_KHR
// 		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
// #endif
// #ifdef VK_USE_PLATFORM_XCB_KHR
//         VK_KHR_XCB_SURFACE_EXTENSION_NAME,
// #endif
// #ifdef VK_USE_PLATFORM_METAL_EXT
//         VK_EXT_METAL_SURFACE_EXTENSION_NAME,
// #endif
// 		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
// 	};

// 	createInfo.ppEnabledExtensionNames = extensions;
// 	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

// 	VkInstance instance = 0;
// 	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

// 	return instance;
// }

// int main(){
//     int rc = glfwInit();
// 	assert(rc);

//     if (glfwVulkanSupported())
//     {
//         glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//         GLFWwindow* window = glfwCreateWindow(1024, 768, "niagara", 0, 0);
// 	    assert(window);

//         VK_CHECK(volkInitialize());

//         VkInstance instance = createInstance();
//         assert(instance);

//         volkLoadInstance(instance);
//         uint32_t size;
//         auto exts = glfwGetRequiredInstanceExtensions(&size);

//         VkSurfaceKHR surface;
//         VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
//         if (err)
//         {
//             return 2;
//         }


//         while (!glfwWindowShouldClose(window))
//         {
//             glfwPollEvents();
//         }

//         glfwDestroyWindow(window);
//     }
// }