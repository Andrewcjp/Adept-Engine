#include "Core\Common.hlsl"
struct PSInput
{
	float4 position : SV_POSITION;
	float4 LastPos :TEXCOORD;
	float4 CurrentPos :TEXCOORD1;
};

Texture2D g_texture : register(t0);
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

cbuffer LastFrameData : register(b3)
{
	row_major matrix LView;
	row_major matrix LProjection;
};

PSInput main(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD, float4 Tangent : TANGENT0
#if WITH_INSTANCING
	, uint id : SV_InstanceID
#endif
)
{
	PSInput result = (PSInput)0;	
#if WITH_INSTANCING
	float4 WorldPos = mul(float4(position.xyz, 1.0f), PrimD[id].Model);
	result.id = id;
#else
	float4 WorldPos = mul(float4(position.xyz, 1.0f), PrimD[0].Model);
#endif	
	float4 final_pos = mul(WorldPos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos;

	float4 lPos = mul(WorldPos, LView);
	lPos = mul(lPos, LProjection);
	result.LastPos = lPos;
	result.CurrentPos = final_pos;
	return result;
}
