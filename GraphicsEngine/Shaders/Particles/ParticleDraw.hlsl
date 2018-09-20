#include "Particle_Common.hlsl"
cbuffer ParticleData : register(b1)
{
	row_major matrix VP;
	float3 CameraRight_worldspace;
	float3 CameraUp_worldspace;
};
cbuffer Index : register(b0)
{
	int index;
};
struct VSData
{
	float4 Pos : SV_POSITION;
};
StructuredBuffer<PosVelo> newPosVelo	: register(t0);	// UAV
static const float particleSize = 1;
VSData VSMain(float4 pos : POSITION)
{
	VSData output = (VSData)0;
	float3 particleCenter_wordspace = newPosVelo[index].pos.xyz;
	float3 vertexPosition_worldspace =
		particleCenter_wordspace
		+ CameraRight_worldspace * pos.x * particleSize
		+ CameraUp_worldspace * pos.y * particleSize;

	output.Pos = mul(float4(vertexPosition_worldspace.xyz, 1.0f), VP);
	return output;
}

//float4 FSMain(VSData input) :SV_Target
//{
//	return float4(0,0,0,0);
//	return float4(newPosVelo[0].pos.xy,0, 1);
//}