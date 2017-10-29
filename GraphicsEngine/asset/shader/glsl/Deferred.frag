#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
vec2 Itexcorrds;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;
uniform sampler2D gTangent;
uniform float far_plane;
uniform vec3 viewPos;
mat3 TBN;
vec3 WorldPos;
#include "Lighting.glh"

#include "Shadow.glh"

const int TYPE_DIRECTIONAL = 0;
const int TYPE_POINT = 1;
const int TYPE_SPOT = 2;


void main()
{        
    // Retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
	WorldPos = FragPos;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
	float AmbientOcclusion = texture(ssao, TexCoords).r;
	vec3 Tangent = texture(gTangent, TexCoords).rgb;
	//consruct the tbn matrix
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
 
    vec3 lighting = Diffuse * 0.2 * AmbientOcclusion; 
    vec3 viewDir  = normalize(viewPos - FragPos);
	//viewDir = normalize(-FragPos);
    for(int i = 0; i < numLights; ++i)
    {
        
        vec3 lightDir = normalize(allLights[i].position - FragPos);
		Diffuse *=allLights[i].Color;

       vec3 specularcol = allLights[i].Color  * Specular;
        
       float distance = length(allLights[i].position - FragPos);

	   float attenuation = 0.01;
	   if(allLights[i].type != TYPE_DIRECTIONAL){
			attenuation = 1.0/(distance*distance);
	   }	   
	   float vis = 1.0;
	   if(allLights[i].HasShadow == 1 ){
		 vis = 1.0 - ShadowCalculationCube(FragPos,allLights[i]);
	   }
	   //vis = 1;
       lighting += ((CalculateColour(Normal,lightDir,Diffuse,specularcol)*attenuation)*vis);
	//	lighting  =vec3(AmbientOcclusion);
    }    
	

	vec3 colorGammaCorrected = pow(lighting, vec3(1.0/2.2));//apply gamma correction
    FragColor = vec4(colorGammaCorrected, 1.0);
}
