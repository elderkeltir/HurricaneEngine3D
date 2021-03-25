#include "VulkanSurface.h"
#include "VulkanBackend.h"
#include "VulkanCamera.h"

#include <GLFW/glfw3.h>

#include <vector>

static double last_xpos;
static double last_ypos;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_W && action == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Move(0, 0, 1);
        }
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Move(0, 0, -1);
        }
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Move(1, 0, 0);
        }
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Move(-1, 0, 0);
        }
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Move(0, -1, 0);
        }
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Move(0, 1, 0);
        }
    }
}
bool firstMouse = true; // TODO: rewrite all this code later. I'm in hurry now QQ
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        last_xpos = xpos;
        last_ypos = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - last_xpos;
    float yoffset = last_ypos - ypos;

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS){
        VulkanBackend * backend = (VulkanBackend*)glfwGetWindowUserPointer(window);
        if (backend){
            backend->GetCamera()->Rotate(xoffset, yoffset);
        }
    }

    last_xpos = xpos;
    last_ypos = ypos;
}

void VulkanSurface::Initialize(VkInstance instance, VkPhysicalDevice physicalDevice){
    r_instance = instance;
    r_physicalDevice = physicalDevice;

    // create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(1280, 720, "VkRender", 0, 0); // TODO: fix hardcodeed values
    assert(m_window);

    // create surface
    VK_CHECK(glfwCreateWindowSurface(r_instance, m_window, NULL, &m_vk_surface));

    glfwSetWindowUserPointer(m_window, r_backend);

    glfwSetKeyCallback(m_window, key_callback);
    glfwSetCursorPosCallback(m_window, cursor_position_callback);
    last_xpos = 0;
    last_ypos = 0;
}

std::vector<const char*> VulkanSurface::GetRequiredExtension(){
    uint32_t glfwExtensionsNum = 0;
	const char ** glfwxtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsNum);
    std::vector<const char*> extensions(glfwExtensionsNum);
    for (size_t i = 0u; i < glfwExtensionsNum; i++){
        extensions[i] = glfwxtensions[i];
    }

    return extensions;
}

bool VulkanSurface::CheckPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) const{
    return glfwGetPhysicalDevicePresentationSupport(r_instance, physicalDevice, queueFamilyIndex);
}

void VulkanSurface::GetWindowsExtent(uint32_t &width, uint32_t &height) const{
    VkSurfaceCapabilitiesKHR surfaceCaps;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r_physicalDevice, m_vk_surface, &surfaceCaps));

    width = surfaceCaps.currentExtent.width;
    height = surfaceCaps.currentExtent.height;
}

bool VulkanSurface::PollWindowEvents() const{
    if (!glfwWindowShouldClose(m_window)){
		glfwPollEvents();

        return true;
    }

    return false;
}

VkSurfaceKHR& VulkanSurface::Vk_surface(){
    return m_vk_surface;
}

VulkanSurface::VulkanSurface(VulkanBackend * backend) :
    r_backend(backend)
{
    // init glfw
    const int rc = glfwInit();
	assert(rc);
}

VulkanSurface::~VulkanSurface(){
    vkDestroySurfaceKHR(r_instance, m_vk_surface, 0);
    glfwDestroyWindow(m_window);
}

VkFormat VulkanSurface::GetSwapchainFormat() const{
	uint32_t formatCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r_physicalDevice, m_vk_surface, &formatCount, 0));
	assert(formatCount > 0);

	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(r_physicalDevice, m_vk_surface, &formatCount, formats.data()));

	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		return VK_FORMAT_R8G8B8A8_UNORM;

	for (uint32_t i = 0; i < formatCount; ++i)
		if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM || formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
			return formats[i].format;

	return formats[0].format;
}