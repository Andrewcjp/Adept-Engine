Texture2D Texture : register(t10);
sampler DefaultSampler :register(s0);


struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 Normal: SV_NORMAL;
	float2 UV: SV_TEXCOORDS;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	return float4(Texture.Sample(DefaultSampler,input.UV).xyz,1.0f);
}