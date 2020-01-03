struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
};
Texture2D BaseTex : register(t1);
Texture2D HeightMap : register(t0);
SamplerState g_sampler : register(s0);
struct FS_OUTPUT
{
	float4 Gpos: SV_Target0;
	float4 GNormal: SV_Target1;
	float4 GAlbedoSpec: SV_Target2;
	float4 GTangent: SV_Target3;
};

FS_OUTPUT main(PSInput input) : SV_TARGET
{
	float4 Sample = HeightMap.SampleLevel(g_sampler, input.uv.xy, 0);
	if (Sample.b > 0.9f)
	{
		discard;
	}

	FS_OUTPUT output = (FS_OUTPUT)0;
	output.Gpos = input.WorldPos;
	output.GNormal = float4(input.Normal.xyz, 0);
	output.GAlbedoSpec = float4(BaseTex.Sample(g_sampler, input.uv).xyz, 1);
	return output;
}