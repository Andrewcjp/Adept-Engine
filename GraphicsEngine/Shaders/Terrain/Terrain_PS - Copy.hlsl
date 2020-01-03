struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
};
Texture2D BaseTex : register(t1);
SamplerState g_sampler : register(s0);
float4 main(PSInput input) : SV_TARGET
{
	float4 Sample = BaseTex.Sample(g_sampler, input.uv.xy);
	return float4(Sample);
}