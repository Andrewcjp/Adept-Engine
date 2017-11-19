#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 vertexTangent_modelspace;
layout (location = 4) in vec3 vertexBitangent_modelspace;//invlaid 
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec4 PositionLightSpace;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out mat3 TBN;
uniform mat3 MV3x3;
void main()
{
    TexCoords = texCoords;
    WorldPos = vec3(model * vec4(pos, 1.0f));
    Normal = normalize((model * vec4(normal,0.0)).xyz);

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
	
	 // model to camera = ModelView
	vec3 vertexTangent_cameraspace = normalize(mat3(model )* vertexTangent_modelspace);
    vec3 vertexBitangent_cameraspace = normalize(mat3(model) * vertexBitangent_modelspace);
    vec3 vertexNormal_cameraspace = normalize( Normal);
    // re-orthogonalize T with respect to N
	vertexTangent_cameraspace = normalize(vertexTangent_cameraspace - dot(vertexTangent_cameraspace, vertexNormal_cameraspace) * vertexNormal_cameraspace);
    vertexBitangent_cameraspace = cross(vertexTangent_cameraspace,vertexNormal_cameraspace);
    // then retrieve perpendicular vector B with the cross product of T and N
	 TBN = transpose(mat3(
		vertexTangent_cameraspace,
		vertexBitangent_cameraspace,
		vertexNormal_cameraspace	
	));
	
	
}