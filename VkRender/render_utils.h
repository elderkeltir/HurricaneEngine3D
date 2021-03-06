#pragma once

#include <cinttypes>
#include <cassert>
#include <volk.h>

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while (0)
	
struct BufferPtr{
	VkBuffer bufferRef;
	VkDeviceMemory memoryRef;
	uint32_t usageType; /*BufferUsageType*/
	VkDeviceSize offset;
	VkDeviceSize size;
	BufferPtr() : bufferRef(nullptr), memoryRef(nullptr), offset(0u), size(0u), usageType(0u) {}
	void Validate() {
		assert(bufferRef && memoryRef);
	}
};
// #include <glm/vec3.hpp> // glm::vec3
// #include <glm/vec4.hpp> // glm::vec4
// #include <glm/mat4x4.hpp> // glm::mat4
// #include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
// #include <glm/ext/matrix_clip_space.hpp> // glm::perspective
// #include <glm/ext/scalar_constants.hpp> // glm::pi