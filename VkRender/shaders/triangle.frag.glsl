#version 450

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec4 color;
void main()
{
	outputColor = color;
	//outputColor = vec4(1.0, 0.0, 1.0, 1.0);
}