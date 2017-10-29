#version 330 core

out vec4 color;
flat in  int UseBack;
in vec3 FrontColour;
in vec3 BackColour;
void main()
{    

	
    if(UseBack == 0)
	{
	  color = vec4(FrontColour,1.0) ;
	}else{
	  color = vec4(BackColour,1.0) ;
	}
//color = vec4(1,1,1,1);
}  