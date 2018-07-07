
struct PSInput
{
	float4 position : SV_POSITION;
	float3 texcoord :TEXCOORD;
};

TextureCube g_texture : register(t0);
SamplerState g_sampler : register(s1);


float4 main(PSInput input) : SV_TARGET
{
	return float4(g_texture.Sample(g_sampler, input.texcoord).xyz,1.0f);
}

