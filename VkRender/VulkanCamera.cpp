#include "VulkanCamera.h"
#include "VulkanBackend.h"
#include "render_utils.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec4.hpp>

const float YAW = -90.0f; // TODO: make separate window class, when get rid of glfw, and rewrite camera class after please >_<
const float PITCH = 0.0f;
const float SPEED = 2000.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

void VulkanCamera::Rotate(float x, float y)
{
    x *= SENSITIVITY;
    y *= SENSITIVITY;

    m_yaw += x;
    m_pitch += y;

    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_direction = glm::normalize(front);

    m_right = glm::normalize(glm::cross(m_direction, glm::vec3(0.f, 1.f, 0.f)));
    m_up = glm::normalize(glm::cross(m_right, m_direction));
}

void VulkanCamera::Move(int x, int y, int z)
{
    if (x)
    {
        m_disp += m_right * (float)(x * SPEED);
    }
    if (y)
    {
        m_disp += m_up * (float)(y * SPEED);
    }
    if (z)
    {
        m_disp += m_direction * (float)(z * SPEED);
    }
}

void VulkanCamera::Update(float dt)
{
    m_position += m_disp * dt;
    m_disp = glm::vec3(0.f, 0.f, 0.f);
}

VulkanCamera::VulkanCamera(VulkanBackend *backend) : 
    r_backend(backend),
    m_width(~0u),
    m_height(~0u),
    m_position(glm::vec3(0.f)),
    m_direction(glm::vec3(0.f)),
    m_up(glm::vec3(0.f)),
    m_right(glm::vec3(0.f)),
    m_disp(glm::vec3(0.f)),
    m_yaw(~0u),
    m_pitch(~0u)
{
}

void VulkanCamera::Initialize(uint32_t width, uint32_t height, const glm::vec3 &position, const glm::vec3 &direction)
{
    m_width = width;
    m_height = height;
    m_position = position;
    m_direction = direction;

    m_up = glm::vec3(0.f, 1.f, 0.f);
    m_right = glm::normalize(glm::cross(m_up, m_direction));
    m_yaw = YAW;
    m_pitch = PITCH;

    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_direction = glm::normalize(front);

    m_right = glm::normalize(glm::cross(m_direction, glm::vec3(0.f, 1.f, 0.f))); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_up = glm::normalize(glm::cross(m_right, m_direction));
}

void VulkanCamera::UpdateExtend(float width, float height)
{
    m_width = width;
    m_height = height;
}

glm::mat4x4 VulkanCamera::GetProjection() const
{
    return glm::perspective(glm::radians(45.0f), m_width / (float)m_height, 0.1f, 75.0f);
}

glm::mat4x4 VulkanCamera::GetView() const
{
    return glm::lookAt(m_position, m_position + m_direction, m_up);
}