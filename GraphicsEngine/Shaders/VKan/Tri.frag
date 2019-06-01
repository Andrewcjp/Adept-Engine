#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(binding = 0) uniform UniformBufferObject {
vec4 color;
} ubo;
layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
vec2 fragTexCoord = vec2(gl_FragCoord.xy*10.0f);
outColor = texture(texSampler, fragTexCoord);
	  //outColor = ubo.color;


}