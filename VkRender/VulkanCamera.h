#pragma once

#include "interfaces/RenderCamera.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class VulkanBackend;

class VulkanCamera : public iface::RenderCamera{
public:
    void Rotate(float x, float y) override;
    void Move(int x, int y, int z) override;
    void Update(float dt) override;

    VulkanCamera(VulkanBackend * backend);
    void Initialize(uint32_t width, uint32_t height, const glm::vec3 &position, const glm::vec3 &direction);
    void UpdateExtend(float width, float height);
    glm::mat4x4 GetProjection() const;
    glm::mat4x4 GetView() const;

private:
    uint32_t m_width;
    uint32_t m_height;
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_disp;
    float m_yaw;
    float m_pitch;

    VulkanBackend * r_backend;
};