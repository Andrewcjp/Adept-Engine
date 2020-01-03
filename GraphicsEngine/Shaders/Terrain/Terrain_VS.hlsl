#include "Core\Common.hlsl"
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
};

Texture2D HeightMap : register(t0);
SamplerState g_sampler : register(s0);

#if WITH_INSTANCING
cbuffer GOConstantBuffer : register(b0)
{
	PrimitiveData PrimD[MAX_INSTANCES];
};
#else
cbuffer GOConstantBuffer : register(b0)
{
	PrimitiveData PrimD[1];
};
#endif

cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
};

PSInput main(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD)
{
	PSInput result = (PSInput)0;
	float4 final_pos = position;
	float4 NewPos = position;
	float4 Sample = HeightMap.SampleLevel(g_sampler, uv.xy, 0);
	NewPos.y = Sample.r*2;
	//final_pos.w = 1.0f;
#if WITH_INSTANCING
	final_pos = mul(float4(NewPos.xyz, 1.0f), PrimD[id].Model);
	result.id = id;
#else
	final_pos = mul(float4(NewPos.xyz, 1.0f), PrimD[0].Model);
#endif	
	result.WorldPos = final_pos;
	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos;
	result.uv = uv.xy;
	result.Normal = normal;

	return result;
}
