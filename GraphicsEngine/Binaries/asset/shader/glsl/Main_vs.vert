#version 420 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 vertexTangent_modelspace;
layout (location = 4) in vec3 vertexBitangent_modelspace; 

layout (std140,binding = 1)uniform UBuffer
{ 
  mat4 model;
  mat4 view;
  mat4 projection;
};
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec4 PositionLightSpace;
out mat3 TBN;
uniform mat3 MV3x3;

uniform vec3 camPos;
uniform mat3 invmodel;

out vec3 TangentViewPos; 
out vec3 TangentFragPos;
out vec3 EyeDirection_cameraspace;
out vec3 tangentdbg;
out vec4 ShadowCoord;
void main()
{
tangentdbg = vertexTangent_modelspace;
    TexCoords = texCoords;
	//fragment position in world space
    WorldPos = (model * vec4(pos, 1.0)).xyz;
	Normal = normalize((model * vec4(normal,0.0)).xyz);	
  gl_Position =  projection *view * model* vec4(pos, 1.0);	
   // gl_Position =   P*V*M* vec4(pos, 1.0);	
	 // model to camera = ModelView
	vec3 vertexTangent_cameraspace = normalize(mat3(model )* vertexTangent_modelspace);
    vec3 vertexBitangent_cameraspace = normalize(vertexBitangent_modelspace);//normalize(mat3(model) * cross(Normal,vertexTangent_modelspace));//normalize(mat3(model) * vertexBitangent_modelspace);
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
	ShadowCoord =  projection *view * model* vec4(pos, 1.0);
	TangentViewPos = TBN * camPos;
	TangentFragPos = TBN * WorldPos;
	
	
}