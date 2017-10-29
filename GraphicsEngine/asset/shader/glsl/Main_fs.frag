#version 420 core


out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
smooth in vec3 Normal;
in mat3 TBN;
in vec4 PositionLightSpace;
in vec3 EyeDirection_cameraspace;
in vec3 TangentViewPos; 
in vec3 TangentFragPos;
in vec4 ShadowCoord;
// material parameters
uniform sampler2D albedoMap;

uniform float far_plane;
uniform int isMap;
uniform int HasNormal;
uniform bool HasDisp;
uniform mat4 view;
uniform sampler2D NormalMap;
uniform sampler2D Displacementmap;
uniform mat4 model;
in vec3 tangentdbg;
int MaxDistance = 500;
uniform vec3 viewPos;
#include "Lighting.glh"

const int TYPE_DIRECTIONAL = 0;
const int TYPE_POINT = 1;
const int TYPE_SPOT = 2;


// lights

vec3 albedo;
float height_scale = 0.06;//make a uniform!
float ShadowCalculationCube(const vec3 fragPos, Light lpos);
float ShadowCalculationDir(const vec3 fragPos, Light lpos);
vec2 PTexCoords;
vec3 TSviewDir;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
float GetFogAmt(const float dist);

uniform int VisShadow;
uniform int FULLBRIGHT;
uniform int IsWater;
float Fogstart = 10.0f;
float fogend = 100.0f;
float FDensity = 0.05f;
uniform float scroll;

#include "Shadow.glh"

vec3 ApplyLight(Light light,vec3 MaterialDiffuseColor,vec3 MaterialSpecularColor,int number){
  
	vec3 TextureNormal_tangentspace = normalize(TBN * (texture( NormalMap, PTexCoords ).rgb*2-1));//from tangent space to camera space

	float distance = length( light.position - WorldPos );
	if(distance > MaxDistance){
		//return vec3(0,0,0);
	}
	
	vec3 n = normalize(TextureNormal_tangentspace);
	if(HasNormal == 0){
		n = normalize(Normal);	
	}
	//lighting 
	vec3 lightdir = normalize(light.position  - WorldPos);	
	if(light.type == TYPE_DIRECTIONAL){
		lightdir = normalize(light.Direction);
	}
    //shadows
	float visibility = 1.0f;
	if(light.HasShadow == 1){	
		visibility = 1.0f - ShadowCalculationCube(WorldPos,light);
		if(light.type == TYPE_DIRECTIONAL){
		visibility = 1.0f - ShadowCalculationDir(WorldPos,light);
		}
	}
	float Attuation = 1;
	if(light.type != TYPE_DIRECTIONAL){
		Attuation = (distance*distance);
	}
	vec3 output = 	(visibility * CalculateColour(n,lightdir,MaterialDiffuseColor,MaterialSpecularColor) * light.Color) / Attuation;
	output = clamp(output,0,1);

    return output;
}


void main()
{			
	
	if(HasDisp == false){
		PTexCoords = TexCoords;
	}
	else
	{	
		PTexCoords = ParallaxMapping(TexCoords, normalize(viewPos - WorldPos));
	}
	if(isMap == 0){
		albedo = texture(albedoMap, PTexCoords).rgb;
	}else{
		albedo = vec3(0,1,1);
	}
	if(IsWater == 1){
		//albedo = mix(albedo,vec3(0,0.8,1),0.25);
		PTexCoords += vec2(0,scroll);
	}
	vec3 MaterialAmbientColor = vec3(0.1) * albedo;

	vec3 linearColor = vec3(0);
   for(int i = 0; i < numLights; ++i){
        linearColor += ApplyLight(allLights[i],albedo,vec3(0.1f),i);
    }
	linearColor = clamp(linearColor,0,1);
	
	 vec3 colorGammaCorrected = pow(linearColor, vec3(1.0f/2.2f));//apply gamma correction
    vec3 color = 		
		MaterialAmbientColor + colorGammaCorrected;
	vec4 eyespacepos = view * vec4(WorldPos,1.0f);
	
	
	if(FULLBRIGHT == 1){
		//color = MaterialDiffuseColor;
	}
	if(VisShadow == 1){
	//	color = vec3(visibility);
	}
	//fog 
	
//	color = mix(color,vec3(0.4,0.4,0.4),GetFogAmt(abs(eyespacepos.z/eyespacepos.w)));

    FragColor = vec4(color, texture(albedoMap, PTexCoords).a);
}

//paralax mappin
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	viewDir = viewDir * TBN;//convert to Tanget space from camera space		
    float height = (texture(Displacementmap, texCoords.xy).r * height_scale);// - (height_scale/2.0);  //bias  	
    return texCoords.xy + viewDir.xy * height;    
} 
float GetFogAmt(const float dist){
	//float result = (fogend-dist)/(fogend-Fogstart);  	
	const float result = exp(-FDensity*dist); 
	return 1.0f - clamp(result, 0.0f, 1.0f); 
}










//float ShadowCalculationDir(const vec3 fragPos, Light lpos){
 // const   vec3 fragToLight = lpos.Direction;//fragPos - lpos.position;
////	vec3 fragToLight = fragPos - allLights[0].position;
//    const float currentDepth = fragPos.z;//length(fragToLight);
//	const vec2 coords = fragPos.xy;
//    float shadow = 0.0f;
 //   const float bias = 0.05f;
//   const  int samples = 20;
//   const  float viewDistance = length(viewPos - fragPos);
//   const  float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0f;
//    for(int i = 0; i < samples; ++i)
 //   {
//	//	float closestDepth = texture(lpos.Shadowflat, fragToLight + gridSamplingDisk[i] * diskRadius).r;
//	if(lpos.DirShadowID == 0){
//		 closestDepth = texture(shadowcubemap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
//		}else if(lpos.DirShadowID == 1){
//		 closestDepth = texture(shadowcubemap2, fragToLight + gridSamplingDisk[i] * diskRadius).r;
//		}
	
//	float closestDepth = texture(shadowdirmap,fragToLight.xy).r;
		
   // closestDepth *= far_plane;   
 //       if(currentDepth - bias > closestDepth)
 // //         shadow += 1.0f;
//}
   // shadow /= float(samples);//average of samples
	//cleanup the low shadow areas
//	if(shadow < 0.25f){
		//return 0.0f;
	//}
	
//	float closestDepth = texture(shadowdirmap,fragToLight.xy).r;
	
 //   return closestDepth;

//



