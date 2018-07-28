#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec4 particlecolor;

// Ouput data
out vec4 color;

uniform sampler2D Texture;

void main(){
	// Output color = color of the texture at the specified UV
	
	color = texture( Texture, UV ) * particlecolor;
	if(color.a < 0.05){
		discard;
	}
//	color = vec4(1,1,1,1);

}