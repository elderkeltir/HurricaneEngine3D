#pragma once

#include <cinttypes>
#include <cassert>
#include <utility>
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

	BufferPtr(BufferPtr&& other){
		std::swap(this->bufferRef, other.bufferRef);
		std::swap(this->memoryRef, other.memoryRef);
		std::swap(this->usageType, other.usageType);
		std::swap(this->offset, other.offset);
		std::swap(this->size, other.size);
	}

	BufferPtr& operator=(BufferPtr&& other){
		std::swap(this->bufferRef, other.bufferRef);
		std::swap(this->memoryRef, other.memoryRef);
		std::swap(this->usageType, other.usageType);
		std::swap(this->offset, other.offset);
		std::swap(this->size, other.size);

		return *this;
	}

	BufferPtr(BufferPtr&) = delete;
	BufferPtr& operator=(BufferPtr&) = delete;
};

struct ImagePtr{
	VkImage imageRef;
	VkDeviceMemory memoryRef;
	VkImageView imageView;
	VkSampler sampler;
	uint32_t usageType; /*BufferUsageType*/
	VkDeviceSize offset;
	VkDeviceSize size;
	ImagePtr() : imageRef(nullptr), memoryRef(nullptr), imageView(nullptr), sampler(nullptr), offset(0u), size(0u), usageType(0u) {}
	void Validate() {
		assert(imageRef && memoryRef);
	}

	ImagePtr(ImagePtr&& other){
		std::swap(this->imageRef, other.imageRef);
		std::swap(this->memoryRef, other.memoryRef);
		std::swap(this->imageView, other.imageView);
		std::swap(this->sampler, other.sampler);
		std::swap(this->usageType, other.usageType);
		std::swap(this->offset, other.offset);
		std::swap(this->size, other.size);
	}

	ImagePtr& operator=(ImagePtr&& other){
		std::swap(this->imageRef, other.imageRef);
		std::swap(this->memoryRef, other.memoryRef);
		std::swap(this->imageView, other.imageView);
		std::swap(this->sampler, other.sampler);
		std::swap(this->usageType, other.usageType);
		std::swap(this->offset, other.offset);
		std::swap(this->size, other.size);

		return *this;
	}

	ImagePtr(ImagePtr&) = delete;
	ImagePtr& operator=(ImagePtr&) = delete;
};
// #include <glm/vec3.hpp> // glm::vec3
// #include <glm/vec4.hpp> // glm::vec4
// #include <glm/mat4x4.hpp> // glm::mat4
// #include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
// #include <glm/ext/matrix_clip_space.hpp> // glm::perspective
// #include <glm/ext/scalar_constants.hpp> // glm::pi