#version 330 core
in vec2 TexCoords;
out vec4 color;
in float sec;
uniform sampler2D text;
uniform vec3 textColor;
uniform int UseColour;
void main()
{    
  //  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
//	vec4 outc = UseColour + texture(text, TexCoords);
	if(sec == 0.0){
	  color = vec4(textColor,1.0) ;
	}else{
	
	  color = vec4(textColor*0.1,1.0) ;
	}
  
	//color = vec4(TexCoords,0,0);
}  