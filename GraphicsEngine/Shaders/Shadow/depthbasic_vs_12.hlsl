#include "Core\Common.hlsl"
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
cbuffer GeoTrans : register(b1)
{
	row_major matrix worldm[6];
}
cbuffer SceneconstantBuffer : register(b2)
{
	row_major matrix ViewP;
	row_major matrix Projection;
	float3 LightPos;
};
[[vk::push_constant]] cbuffer VIData : register(b3)
{
	int Offset;
}
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float3 LightPos: TEXCOORD0;
	float3 WorldPos:NORMAL0;
};


VS_OUTPUT main(float4 pos : POSITION, float4 normal : NORMAL0, float3 uv : TEXCOORD0
#if USE_VIEWINST
	, int V : SV_ViewID
#endif
#if WITH_INSTANCING
	, uint id : SV_InstanceID
#endif
)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
#if WITH_INSTANCING
	float4 final_pos = mul(float4(pos.xyz, 1.0), PrimD[id].Model);
#else
	float4 final_pos = mul(float4(pos.xyz, 1.0), PrimD[0].Model);
#endif
	output.WorldPos = final_pos.xyz;

#if USE_VIEWINST
	output.pos = mul(float4(final_pos.xyz, 1.0), worldm[Offset+V]);
	output.slice = Offset + V;
#else
#if VS_WORLD_OUTPUT
	output.pos = mul(float4(final_pos.xyz, 1.0), worldm[Offset]);
#else
	output.pos = final_pos;
#endif	
	output.slice = Offset;
#endif	
	output.LightPos = LightPos;
	return output;
}
