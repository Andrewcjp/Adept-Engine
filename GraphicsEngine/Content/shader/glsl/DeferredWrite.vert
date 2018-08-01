#version 420 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 Vtangent;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec3 Tangent;
out mat3 TBN;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){

    TexCoords = texCoords;

    WorldPos = (model * vec4(pos, 1.0)).xyz;
	Normal = normalize((model * vec4(normal,0.0)).xyz);	

    gl_Position =  projection *view * model* vec4(pos, 1.0);	
	Tangent = normalize(mat3(model) * Vtangent);
	//create the TBN matrix 
	vec3 vtangent = normalize(Tangent);
    vec3 vbitangent = normalize(cross(Normal,Tangent));
    vec3 vnormal = normalize( Normal);
	
    // re-orthogonalize T with respect to N
	vtangent = normalize(vtangent - dot(vtangent, vnormal) * vnormal);
    vbitangent = cross(vtangent,vnormal);
	
	 TBN = transpose(mat3(
		vtangent,
		vbitangent,
		vnormal	
	));
		
	
}