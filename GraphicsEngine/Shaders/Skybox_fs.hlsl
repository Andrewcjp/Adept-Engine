
struct PSInput
{
	float4 position : SV_POSITION;
	float3 texcoord :TEXCOORD;
};

TextureCube g_texture : register(t0);
SamplerState g_sampler : register(s1);


float4 main(PSInput input) : SV_TARGET
{
	return float4(g_texture.SampleLevel(g_sampler, input.texcoord,0).xyz,1.0f);
}

