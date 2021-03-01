#include "render_utils.h"
#include <volk.h>

// forward declaration
class GLFWwindow;

// class to manage window and related surface.
class Surface{
public:
    void Initialize(const VkInstance &instance);
    uint32_t GetRequiredExtension(const char ** extensions) const;
    bool CheckPresentationSupport(const VkPhysicalDevice &physicalDevice, uint32_t queueFamilyIndex) const;
    void GetWindowsExtent(uint32_t &width, uint32_t &height) const;
    bool PollWindowEvents() const;

    VkSurfaceKHR& Vk_surface();

    ~Surface();
private:
    VkSurfaceKHR m_vk_surface;
    GLFWwindow* m_window;
    VkInstance r_instance;
};