#include "Particle_Common.hlsl"
cbuffer ParticleData : register(b1)
{
    row_major matrix VP;
    float4 CameraRight_worldspace;
    float4 CameraUp_worldspace;
};
PUSHCONST cbuffer Index : register(b0)
{
    int index;
};
struct VSData
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORDS;
};
StructuredBuffer<PosVelo> newPosVelo : register(t0);
Texture2D Tex : register(t1);
SamplerState g_sampler : register(s0);
//static const float particleSize = 0.5f;
VSData VSMain(float4 pos : POSITION)
{
    VSData output = (VSData) 0;
    float3 particleCenter_wordspace = newPosVelo[index].pos.xyz;
    const float particleSize = newPosVelo[index].Size;
    float3 vertexPosition_worldspace =
		particleCenter_wordspace
		+ CameraRight_worldspace.xyz * pos.x * particleSize
		+ CameraUp_worldspace.xyz * pos.y * particleSize;

    output.Pos = mul(float4(vertexPosition_worldspace.xyz, 1.0f), VP);
    output.UV = (pos.xy + 1.0f) / 2.0f;
    output.UV.y = -output.UV.y;

    return output;
}

float4 FSMain(VSData input) : SV_Target
{
    float4 Colour = Tex.Sample(g_sampler, input.UV);
    if (Colour.a < 0.1f)
    {
        discard;
    }
    //if (newPosVelo[index].Lifetime <= 0.0f)
    //{
    //   discard;
    //}
    Colour.a += 0.3f;
    return Colour;
}