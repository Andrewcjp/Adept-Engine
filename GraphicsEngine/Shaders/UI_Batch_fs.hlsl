
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 FrontColour:TEXCOORD;
	float3 BackColour:TEXCOORD1;
	uint Back : TEXCOORD2;
};
SamplerState Sampler : register(s0);
Texture2D texColour : register(t0);

float4 main(VS_OUTPUT input) : SV_Target
{
#if USE_TEXTURED
	float4 output = texColour.Sample(Sampler, input.BackColour.xy);
	return output;
#else
	if (input.Back == 0 )
	{
		return float4(input.FrontColour.xyz, 0.0);
	}
	else
	{
		return float4(input.BackColour.xyz,0.0);
	}
#endif
}
