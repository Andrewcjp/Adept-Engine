Texture2D texColour : register(t0);
SamplerState defaultSampler : register (s0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};


float4 main(VS_OUTPUT input) : SV_Target
{
	//return float4(1,1,1,1);
	float4 output = texColour.Sample(defaultSampler, input.uv);
	//output.a = 0.25f;
	return output;
	
}
