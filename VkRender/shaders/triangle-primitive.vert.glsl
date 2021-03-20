#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position + vec3(0, -0.95, 0.5), 1.0);
}