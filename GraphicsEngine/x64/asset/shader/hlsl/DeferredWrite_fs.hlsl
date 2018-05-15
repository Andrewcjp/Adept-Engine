Texture2D texColour : register( t0 );
SamplerState defaultSampler : register ( s0 );

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
};
struct FS_OUTPUT
{
	float4 Gpos: SV_Target0;
	float4 GNormal: SV_Target1;
	float4 GAlbedoSpec: SV_Target2;
	float4 GTangent: SV_Target3;
};
FS_OUTPUT main(PSInput input ) 
{
	FS_OUTPUT output = (FS_OUTPUT)0;
	output.Gpos = input.WorldPos;
	output.GNormal = float4(input.Normal.xyz,1.0f);//todo: remove this exta compoenent
	output.GAlbedoSpec = float4(texColour.Sample(defaultSampler, input.uv).xyz, 1.0f);
    return output;
}
