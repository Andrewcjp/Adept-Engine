#version 420 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 vertexTangent_modelspace;
layout (location = 4) in vec3 vertexBitangent_modelspace; 

out vec3 WorldPos;

uniform mat4 MVP;

void main()
{
    gl_Position =  MVP* vec4(pos, 1.0);	
	
}