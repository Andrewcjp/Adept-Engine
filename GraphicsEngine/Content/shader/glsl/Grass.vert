#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec2 inuv;
layout (location = 5) in vec2 vpos;
layout (location = 7) in vec3 colour;

uniform mat4 projMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;                                                                           
uniform mat4 normalMatrix;
vec4 adjustedpos;
out vec2 vTexCoord;
#define MAXInstances 200
#define MAXUniqueRotations 200

uniform mat4 Rotations[MAXInstances];
/*uniform RotBuffer
{ 
  mat4 urot;
}UBufferRot[1];*/
out vec3 color;
int instid = 0;

void main()
{
	vTexCoord = inuv;
	
	instid = gl_InstanceID;
	/*if(gl_InstanceID > MAXUniqueRotations){			
			instid = int(MAXUniqueRotations* sin(gl_InstanceID - MAXUniqueRotations));			
			//repeat the values so that grass is animated at a lower cost
			// if wind is added this system becmoes usless as only wind position and force would be passed to the shader a few times (array)
			//rather than an entire mat4x4 for each grass object.
	}*/
	if(gl_InstanceID > MAXUniqueRotations)
	{			
			instid = int(MAXUniqueRotations* sin(gl_InstanceID - MAXUniqueRotations));			
	}
	instid = clamp(instid,0,MAXUniqueRotations-1);

	//adjustedpos = vec4(inPosition,1.0)*Rotations[instid] + vec4(Transforms[gl_InstanceID].x, 0, Transforms[gl_InstanceID].y,1.0);
	
	adjustedpos = (vec4(inPosition,1.0)*Rotations[instid]) + vec4(vpos.x, 0.0, vpos.y,1.0);
	
	gl_Position =  projMatrix * viewMatrix * modelMatrix * vec4(adjustedpos);

}