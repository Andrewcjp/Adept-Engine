Texture2D texColour : register(t0);
SamplerState g_sampler: register (s0);

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
	float2 LightData : TEXCOORD1;
};

struct FS_OUTPUT
{
	float4 Gpos: SV_Target0;
	float4 GNormal: SV_Target1;
	float4 GAlbedoSpec: SV_Target2;
	float4 GTangent: SV_Target3;
};
//Declares
FS_OUTPUT main(PSInput input)
{
	FS_OUTPUT output = (FS_OUTPUT)0;
	output.Gpos = input.WorldPos;
	
	float3 texturecolour = float3(0, 0, 0);
	float3 Normal = input.Normal.xyz;
	//Insert Marker
	texturecolour = Diffuse;
	output.GNormal = float4(Normal.xyz, input.LightData.g);
	output.GAlbedoSpec = float4(texturecolour, input.LightData.r);
	return output;
}
