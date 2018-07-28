#version 430

layout (location = 0) uniform mat4 modelview;	//modelview matrix
layout (location = 1) uniform mat4 MVP;	//projection matrix
layout (location = 2) uniform vec4 lightpos;	//light position
layout (location = 3) uniform mat4 lightmatrix;	//light matrix
layout (location = 4) uniform mat4 lightproj;	//light projection matrix

layout (location = 0) in vec4 vertposition;	//vertex attribute: position

out vec4 outNormal;		//output: normal
out vec4 lightvec;		//output: light vector
out vec4 viewvec;		//output: view vector
out vec2 outUV;			//output: texcoords
out vec4 outPosInLight;	//output: vertex position in light space
out vec4 position;
uniform mat4 model;
uniform int Ispoint;
void main()
{	
	if(Ispoint == 1){
		gl_Position = model*vertposition;
	}else{
		gl_Position = MVP*model*vertposition;
	}
	position = vertposition;

}
