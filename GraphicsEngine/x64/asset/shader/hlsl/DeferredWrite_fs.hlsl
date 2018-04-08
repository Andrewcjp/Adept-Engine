Texture2D texColour : register( t0 );
SamplerState defaultSampler : register ( s0 );

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float3 uv : TEXCOORD0;
};
struct FS_OUTPUT
{
	float3 Gpos: COLOR0;
	float3 GNormal: COLOR1;
	float4 GAlbedoSpec: COLOR2;
	float3 GTangent: COLOR3;
}
FS_OUTPUT main( VS_OUTPUT input ) /*: SV_Target*/
{
	FS_OUTPUT output = (FS_OUTPUT)0;
	output.Gpos = pos.xyz;
	output.GNormal = normal.xyz;
	output.GAlbedoSpec = float4(texColour.Sample(defaultSampler, input.uv), 0.0f);
    return output;
}
