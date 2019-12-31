#include "core/Common.hlsl"
#define WITH_INSTANCING 1
struct GSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
};

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

GSInput main(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD
#if WITH_INSTANCING
	, uint id : SV_InstanceID
#endif
)
{
	GSInput result = (GSInput)0;
	float4 final_pos = position;
#if WITH_INSTANCING
	final_pos = mul(float4(position.xyz, 1.0f), PrimD[id].Model);
	//result.id = id;
#else
	final_pos = mul(float4(position.xyz, 1.0f), PrimD[0].Model);
#endif	
	result.WorldPos = final_pos;
	result.position = final_pos;
	result.uv = uv.xy;
	result.Normal = normal;
	return result;
}