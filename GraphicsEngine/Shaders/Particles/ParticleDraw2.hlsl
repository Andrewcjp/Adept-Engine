#include "Particle_Common.hlsl"
struct VSData
{
	float4 Pos : SV_POSITION;
};
StructuredBuffer<PosVelo> newPosVelo	: register(t0);	// UAV
VSData VSMain(float4 pos : POSITION)
{
	VSData output = (VSData)0;
	output.Pos = pos;
	return output;
}

float4 FSMain(VSData input) :SV_Target
{
	return float4(newPosVelo[0].pos.w,0 ,0, 1);
}