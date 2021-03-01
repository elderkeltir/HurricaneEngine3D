#include "Surface.h"

#include <GLFW/glfw3.h>

void Surface::Initialize(const VkInstance &instance){
    r_instance = instance; // do not own instance, just ref it
    // init glfw
    const int rc = glfwInit();
	assert(rc);

    // create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(1024, 768, "VkRender", 0, 0); // TODO: fix hardcodeed values
    assert(m_window);

    // create surface
    VK_CHECK(glfwCreateWindowSurface(r_instance, m_window, NULL, &m_vk_surface));
}

uint32_t Surface::GetRequiredExtension(const char ** extensions) const{
    uint32_t glfwExtensionsNum = 0;
	*extensions = *glfwGetRequiredInstanceExtensions(&glfwExtensionsNum);
}

bool Surface::CheckPresentationSupport(const VkPhysicalDevice &physicalDevice, uint32_t queueFamilyIndex) const{
    return glfwGetPhysicalDevicePresentationSupport(r_instance, physicalDevice, queueFamilyIndex);
}

void Surface::GetWindowsExtent(uint32_t &width, uint32_t &height) const{

}

bool Surface::PollWindowEvents() const{
    if (!glfwWindowShouldClose(m_window)){
		glfwPollEvents();

        return true;
    }

    return false;
}

VkSurfaceKHR& Surface::Vk_surface(){
    return m_vk_surface;
}

Surface::~Surface(){
    vkDestroySurfaceKHR(r_instance, m_vk_surface, 0);
    glfwDestroyWindow(m_window);
}