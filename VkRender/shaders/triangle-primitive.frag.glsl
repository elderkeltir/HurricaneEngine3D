#version 450

layout(location = 0) out vec4 outputColor;

layout( push_constant ) uniform ColorBlock {
	layout(offset = 0) vec4 Color;
	// vec4 Color;
} PushConstant;

void main()
{
	outputColor = PushConstant.Color;
}