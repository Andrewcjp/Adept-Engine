#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(binding = 0) uniform UniformBufferObject {
vec4 color;
} ubo;

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
	  outColor = ubo.color;
    outColor = vec4(fragColor, 1.0);
	outColor.r = ubo.color.r;
}