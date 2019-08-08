#include "../Source/Core/Rendering/Core/LightInterop.h"
static const float MAX_REFLECTION_LOD = 11.0;
//PBR functions!
static const float PI = 3.14159265359;
float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlick_Roughness(float cosTheta, float3 F0, float Roughness)
{
	return F0 + (max(float3(1.0 - Roughness, 1.0 - Roughness, 1.0 - Roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 Phong_Diffuse(float3 MaterialDiffuseColor, float3 LightDir, float3 Normal)
{
	float diffu = max(dot(Normal, LightDir), 0.0);
	return MaterialDiffuseColor * diffu;
}

float3 GetAmbient_CONST()
{
	const float Value = 0.1f;
	return float3(Value, Value, Value);
}

float3 GetAmbient(float3 Normal, float3 View, float3 Diffusecolor,float Roughness,float Metal,float3 IRData,float3 SpecularRefl,float2 envBRDF)
{
	Roughness = clamp(Roughness, 0.0f, 1.0f);
	Metal = clamp(Metal, 0.0f, 1.0f);
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, Diffusecolor, Metal);
	float3 F = fresnelSchlick_Roughness(max(dot(Normal, View), 0.0), F0, Roughness);
	float3 kD = 1.0 - F;
	kD *= 1.0 - Metal;
	float3 diffuse = IRData * Diffusecolor;
	float3 Specular = SpecularRefl * (F * envBRDF.x + envBRDF.y);
	float3 ambient = (kD * diffuse+ Specular);
	return ambient;
}

float3 CalcColorFromLight_FALLBACK(Light light,float3 Diffusecolor,float3 FragPos,float3 normal)
{
	float3 LightDirection = float3(0, 1, 0);
	float distanceToLight = length(light.LPosition - FragPos);
	float attenuation = 1.0f;

	if (light.type == 1)
	{
		LightDirection = normalize(light.LPosition - FragPos);
		attenuation = 1.0 / (1.0 + 0.001 * pow(distanceToLight, 2));
	}
	else
	{
		LightDirection = -light.Direction;
	}

	float3 Diffusecolour = Phong_Diffuse(Diffusecolor, LightDirection, normal) * light.color;
	return Diffusecolour * attenuation;
}

float3 CalcColorFromLight(Light light, float3 Diffusecolor, float3 FragPos, float3 normal,float3 CamPos,float roughness,float Metalic)
{
	roughness = clamp(roughness, 0.0f, 1.0f);
	Metalic = clamp(Metalic, 0.0f, 1.0f);
	float3 LightDirection = float3(0, 1, 0);
	float distanceToLight = length(light.LPosition - FragPos);
	if (distanceToLight > light.Range)
	{
		return float3(0, 0, 0);
	}

	float3 ViewDir = normalize(CamPos - FragPos);
	if (light.type == 1)
	{
		LightDirection = normalize(light.LPosition - FragPos);
		//attenuation = 1.0 / (1.0 + 0.001 * pow(distanceToLight, 2));
	}
	else
	{
		LightDirection = -light.Direction;
	}

	float3 Half = normalize(ViewDir + LightDirection);

	float attenuation = 1.0 / (distanceToLight * distanceToLight);
	float3 radiance = light.color * attenuation;
	roughness = max(roughness, 0.001);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, Diffusecolor, Metalic);

	// cook-torrance brdf
	float NDF = DistributionGGX(normal, Half, roughness);
	float G = GeometrySmith(normal, ViewDir, LightDirection, roughness);
	float3 F = fresnelSchlick(max(dot(Half, ViewDir), 0.0), F0);
	
	
	float3 kS = F;
	float3 kD = float3(1.0,1.0,1.0) - kS;
	kD *= 1.0 - Metalic;
	
	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, ViewDir), 0.0) * max(dot(normal, LightDirection), 0.0);
	float3 specular = numerator / max(denominator, 0.01);
	//return specular;
	// add to outgoing radiance Lo
	float NdotL = max(dot(normal, LightDirection), 0.0);

	return (kD * Diffusecolor / PI + specular) * radiance * NdotL;
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}