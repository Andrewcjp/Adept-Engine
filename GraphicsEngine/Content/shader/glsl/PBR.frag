#version 330 core
uniform samplerCube shadowcubemap;
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;
in vec4 PositionLightSpace;
// material parameters
uniform sampler2D albedoMap;
uniform float ao;
uniform float far_plane;
uniform int isMap;
uniform int HasNormal;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D NormalMap;
//lights
#include "Shadow.glh"
// lights

uniform vec3 camPos;
uniform float exposure;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float ShadowCalculation(vec3 fragPos);

void main()
{		
		
	vec3 albedo;
	if(isMap == 0){
		albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
	}else{
		albedo = vec3(0,1,1);
	}
	float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
	//v is inverted?
	vec3 TextureNormal_tangentspace = normalize((texture( NormalMap, TexCoords).rgb)*2-1);
    vec3 N = normalize(Normal);
	if(HasNormal == 1){
		N = TextureNormal_tangentspace;
		//albedo = vec3(1,1,1);
	}else{
		
	}
	
    vec3 V = normalize(camPos - WorldPos);
	if(HasNormal == 1){
	V = normalize(V*TBN);
	}
    vec3 R = reflect(-V, N); 


    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(allLights[i].position - WorldPos);
		if(HasNormal == 1){
		L = normalize(L * TBN);//hmmm
		}
        vec3 H = normalize(V + L);
		if(HasNormal == 1){
		H = normalize(H * TBN);
		}
        float distance = length(allLights[i].position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = allLights[i].color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 brdf = nominator / denominator;
        
        // kS is equal to Fresnel value
        vec3 kS = F;

        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

       
        Lo += (kD * albedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    

    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping remap to LDR since 1988
    color = color / (color + vec3(1.0));
//	color = color * 1-ShadowCalculation(WorldPos);
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 
	//color = N;
	//color = albedo;
    FragColor = vec4(color, 1.0);
	//FragColor = vec4(vec3(1- ShadowCalculation(WorldPos)),1.0);
	
}
