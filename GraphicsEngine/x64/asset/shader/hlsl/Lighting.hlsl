#define MAX_LIGHT 4
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


float4 CalcLightingColor(float3 MaterialDiffuseColor, float3 vLightPos, float3 vLightDir, float3 vLightColor, float4 vFalloffs, float3 vPosWorld, float3 vPerPixelNormal)
{

	float diffu = max(dot(vPerPixelNormal, vLightDir), 0.0);//diffuse
	float spec = 0.0f;
	float3 viewPos = float3(0, 0, 0);
	if (diffu > 0.0f)
	{
		/*float3 viewDir = normalize(viewPos - vPosWorld);
		float3 reflectDir = reflect(-lightdir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);*/
	}
	MaterialDiffuseColor *= vLightColor;

	return  float4(MaterialDiffuseColor * diffu, 1.0);// +(MaterialSpecularColor*spec);
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
	float attenuation = 1.0 / (1.0 + 0.001 * pow(distanceToLight, 2));

	if (light.type == 1)
	{
		LightDirection = normalize(light.LPosition - FragPos);
	}
	else
	{
		LightDirection = -light.Direction;
	}

	float3 Diffusecolour = Phong_Diffuse(Diffusecolor, LightDirection, normal) * light.color;
	return Diffusecolour *attenuation;
}