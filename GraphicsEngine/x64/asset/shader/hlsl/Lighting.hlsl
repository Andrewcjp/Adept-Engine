struct Light
{
	float3 LPosition;
	float3 color;
	float3 Direction;
	row_major matrix LightVP;
	int type;//type 1 == point, type 0 == directional, tpye 2 == spot
	int ShadowID;
	int DirShadowID;
	int HasShadow;
};
//PBR functions!
const float PI = 3.14159265359;
float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
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


float3 Phong_Diffuse(float3 MaterialDiffuseColor, float3 LightDir, float3 Normal)
{
	float diffu = max(dot(Normal, LightDir), 0.0);//diffuse
	return MaterialDiffuseColor * diffu;
}

float3 GetAmbient()
{
	return float3(0.2, 0.2, 0.2);
}


float3 CalcColorFromLight(Light light,float3 Diffusecolor,float3 FragPos,float3 normal)
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