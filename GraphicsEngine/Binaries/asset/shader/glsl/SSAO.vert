#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

void main(){
    //TexCoords = texCoords;
	TexCoords = (pos.xy+vec2(1,1))/2.0;
    gl_Position =  vec4(pos, 1.0);	
}
