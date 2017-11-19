#version 330 core
layout (location = 0) in vec2 vertex; // vec2 pos & vec2 texure coods
layout (location = 1) in vec3 col;

out vec3 colr;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
	colr = col;
}  