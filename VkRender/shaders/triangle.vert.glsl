#version 450
#extension GL_ARB_separate_shader_objects : enable

/*const vec3 vertices[] = 
{
	vec3(0, 0.5, 0),
	vec3(0.5, -0.5, 0),
	vec3(-0.5, -0.5, 0),
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
}*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec2 textureCoords;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position + vec3(0, -0.95, 0.5), 1.0);
	textureCoords = texcoord;
	//gl_Position = vec4(position + vec3(0, -0.95, 0.5), 1.0);

	//position = vec4(normal * 0.5 + vec3(0.5), 1.0);
}