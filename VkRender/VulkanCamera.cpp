#include "VulkanCamera.h"
#include "VulkanBackend.h"
#include "render_utils.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec4.hpp>

#include <cstdio> // TODO

void VulkanCamera::Rotate(float x, float y){
    printf("Rotate [%f][%f] before dir=(%f,%f,%f), up=(%f,%f,%f), right=(%f,%f,%f)\n",x, y, vec3log(m_direction), vec3log(m_up), vec3log(m_right));
    if (y){
        float dy = y / m_height;
        //dy = dy * glm::pi<float>() * 2.f;
        m_direction = glm::rotateX(-glm::vec4(m_direction, 0.f), dy * 0.01f);

        //m_direction = glm::rotate(glm::mat4(1.0f), dy, m_right) * glm::vec4(m_direction, 1.f);
        m_up = glm::normalize(-glm::cross(m_direction, m_right));
    }
    else if (x && 0){
        float dx = x / m_width;

        //m_direction = glm::rotate(glm::mat4(1.0f), dx, m_up) * glm::vec4(m_direction, 1.f);
        m_direction = glm::rotateY(glm::vec4(m_direction, 0.f), dx * 0.01f);
        m_right = glm::normalize(-glm::cross(m_direction, m_up));
    }
    printf("Rotate after dir=(%f,%f,%f), up=(%f,%f,%f), right=(%f,%f,%f)\n", vec3log(m_direction), vec3log(m_up), vec3log(m_right));
}

void VulkanCamera::Move(int x, int y, int z){
    const float moveSpeed = 0.25f;
    if (x){
        m_position = m_position + m_right * (float)(x * moveSpeed);
    }
    if (y){
        m_position = m_position + m_up * (float)(y * moveSpeed);
    }
    if (z){
        m_position = m_position + m_direction * (float)(z * moveSpeed);
    }
}

VulkanCamera::VulkanCamera(VulkanBackend * backend) :
    r_backend(backend)
{

}

void VulkanCamera::Initialize(uint32_t width, uint32_t height, const glm::vec3 &position, const glm::vec3 &direction){
    m_width = width;
    m_height = height;
    m_position = position;
    m_direction = glm::normalize(direction);
    m_right = glm::vec3(-1.f, 0.f, 0.f);
    m_up = glm::vec3(0.f, 1.f, 0.f);
}

void VulkanCamera::UpdateExtend(float width, float height){
    m_width = width;
    m_height = height;
}

glm::mat4x4 VulkanCamera::GetProjection() const{
    return glm::perspective(glm::radians(45.0f), m_width / (float)m_height, 0.1f, 10.0f);
}

glm::mat4x4 VulkanCamera::GetView() const{
    return glm::lookAt(m_position, m_position + m_direction, m_up);
}