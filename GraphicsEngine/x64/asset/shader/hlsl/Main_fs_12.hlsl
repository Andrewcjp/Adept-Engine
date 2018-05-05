#define SHADOW_DEPTH_BIAS 0.005f
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

cbuffer LightBuffer : register(b1)
{
	Light lights[MAX_LIGHT];
	//float3 LPosition;
	//float3 color;
	//float3 Direction;
	//row_major matrix LightVP;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD0;
	float4 WorldPos:TANGENT0;
};

Texture2D g_texture : register(t0);

Texture2D g_Shadow_texture : register(t1);
//Texture2D g_Shadow_texture2 : register(t2);
TextureCube g_Shadow_texture2 : register(t2);
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
float GetShadow(float4 pos)
{
	float4 vLightSpacePos = pos;
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].view);
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].projection);
	vLightSpacePos = mul(vLightSpacePos, lights[0].LightVP);
	vLightSpacePos.xyz /= vLightSpacePos.w;
	float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
	vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;
	float bias = 0.005;
	if (g_Shadow_texture.Sample(g_sampler, vShadowTexCoord.xy).r < (vLightSpacePos.z - bias))
	{
		return 1.0f;
	}
	return 0.0f;
}
float4 CalcUnshadowedAmountPCF2x2(int lightid, float4 vPosWorld)
{
	// Compute pixel position in light space.
float4 vLightSpacePos = vPosWorld;
vLightSpacePos = mul(vLightSpacePos, lights[lightid].LightVP);

vLightSpacePos.xyz /= vLightSpacePos.w;

// Translate from homogeneous coords to texture coords.
float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;

// Depth bias to avoid pixel self-shadowing.
float vLightSpaceDepth = vLightSpacePos.z - SHADOW_DEPTH_BIAS;

// Find sub-pixel weights.//todo: shader define!
float2 vShadowMapDims = float2(1024, 1024); // need to keep in sync with .cpp file
float size = 1.0f;
float4 vSubPixelCoords = float4(size, size, size, size);
vSubPixelCoords.xy = frac(vShadowMapDims * vShadowTexCoord);
vSubPixelCoords.zw = 1.0f - vSubPixelCoords.xy;
float4 vBilinearWeights = vSubPixelCoords.zxzx * vSubPixelCoords.wwyy;

// 2x2 percentage closer filtering.
float2 vTexelUnits = 1.0f / vShadowMapDims;
float4 vShadowDepths;
if (lightid == 0)
{
	vShadowDepths.x = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord);
	vShadowDepths.y = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f));
	vShadowDepths.z = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y));
	vShadowDepths.w = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits);
}
else
{
	/*vShadowDepths.x = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord);
	vShadowDepths.y = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f));
	vShadowDepths.z = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y));
	vShadowDepths.w = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits);*/
}
// What weighted fraction of the 4 samples are nearer to the light than this pixel?
float4 vShadowTests = (vShadowDepths >= vLightSpaceDepth) ? 1.0f : 0.0f;
return dot(vBilinearWeights, vShadowTests);
}

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
float ShadowCalculationCube(const float3 fragPos, Light lpos);
float4 main(PSInput input) : SV_TARGET
{
	float3 texturecolour = g_texture.Sample(g_sampler, input.uv);
	float4 output = float4(0, 0, 0, 0);// float4(texturecolour.xyz, 1);
	float4 falloff = (0, 1000, 0.0f, 10.0f);
	for (int i = 0; i < MAX_LIGHT; i++)
	{
		float3 dir = -lights[i].Direction;
		float attenuation = 1;
		if (lights[i].type == 1)
		{
			float distanceToLight = length(lights[i].LPosition - input.WorldPos);
			dir = normalize(lights[i].LPosition - input.WorldPos);
			//float Attuation = (distance*distance);
			attenuation = 1.0 / (1.0 + 0.001 * pow(distanceToLight, 2));
			/*if (distanceToLight > 30)
			{
				continue;
			}*/
		}
		float4 colour = CalcLightingColor(texturecolour, lights[i].LPosition, dir, lights[i].color, falloff, input.WorldPos.xyz, normalize(input.Normal.xyz))* attenuation;
		if (lights[i].HasShadow && lights[i].type == 0)
		{
			colour *= CalcUnshadowedAmountPCF2x2(i, input.WorldPos);
		}
		/*if (lights[i].HasShadow && lights[i].type == 0)
		{
			colour = ShadowCalculationCube( input.WorldPos, lights[i]);
		}*/
		//colour = 1.0f - ShadowCalculationCube(input.WorldPos, lights[0]);
		output += colour;
	}
	// output = float4(lights[1].color * 10, 1.0);
	float AbFactor = 0.2f;
	float4 ambeint = float4(texturecolour,1.0) * float4(AbFactor, AbFactor, AbFactor, AbFactor);
	float Shadow = CalcUnshadowedAmountPCF2x2(0, input.WorldPos);
	float4 GammaCorrected = ambeint + (output);// pow(output, float4(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
	float gamma = 1.0f / 2.2f;
	//	GammaCorrected = pow(GammaCorrected, float4(gamma, gamma, gamma, gamma));
	return GammaCorrected;
}


float ShadowCalculationCube(const float3 fragPos, Light lpos)
{
	// Get vector between fragment position and light position
	float3 fragToLight = fragPos - float3(0, 5, 20);// lpos.LPosition;

	float currentDepth = length(fragToLight);
	//if (currentDepth > MaxShadowDistance)
	//{
	//	//	return 0.0f;
	//}
	float shadow = 0.0f;
	float bias = 0.01f;
	int samples = 1;
	/*float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0f + (viewDistance / far_plane)) / 25.0f;*/
	float far_plane = 500.0f;

	float closestDepth = 0;
	closestDepth = g_Shadow_texture2.Sample(g_Clampsampler, fragToLight).r;

	closestDepth *= far_plane;
	if (currentDepth - bias > closestDepth)
	{
		return 1.0f;
	}
	
	//shadow /= float(samples);//average of samples
	//						 //cleanup the low shadow areas
	/*if (shadow < 0.25f)
	{
		return 0.0f;
	}*/
	return 0.0f;
}



//float4 CalcLightingColor(float3 vLightPos, float3 vLightDir, float4 vLightColor, float4 vFalloffs, float3 vPosWorld, float3 vPerPixelNormal)
//{
//	float3 vLightToPixelUnNormalized = vPosWorld - vLightPos;
//
//	// Dist falloff = 0 at vFalloffs.x, 1 at vFalloffs.x - vFalloffs.y
//	float fDist = length(vLightToPixelUnNormalized);
//
//	float fDistFalloff = 1;//saturate((vFalloffs.x - fDist) / vFalloffs.y);
//
//						   // Normalize from here on.
//	float3 vLightToPixelNormalized = vLightToPixelUnNormalized / fDist;
//
//	// Angle falloff = 0 at vFalloffs.z, 1 at vFalloffs.z - vFalloffs.w
//	float fCosAngle = dot(vLightToPixelNormalized, vLightDir / length(vLightDir));
//	float fAngleFalloff = saturate((fCosAngle - vFalloffs.z) / vFalloffs.w);
//
//	// Diffuse contribution.
//	float fNDotL = saturate(-dot(vLightToPixelNormalized, vPerPixelNormal));
//
//	return vLightColor * fNDotL;// *fDistFalloff;// *fAngleFalloff;// *fDistFalloff;// *fAngleFalloff;
//}