#version 330 core

out vec4 color;
in vec3 colr;

uniform vec3 textColor;

void main()
{    
   color = vec4(colr, 1.0);
}  