#include "Core/Common.hlsl"
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
	INSTANCINGPSDATA
};
struct FS_OUTPUT
{
	float4 Gpos: SV_Target0;
	float4 GNormal: SV_Target1;
	float4 GAlbedoSpec: SV_Target2;
	float4 GTangent: SV_Target3;
};

#include "Core/SamplerFeedBack.hlsl"

FEEDBACK_BIND;
SamplerState Default:register(s0);
Texture2D Diffuse:register(t0);
FEEDBACKTEXTURE(SFS_TYPE_MINMIP) g_feedback : register(u0);

[earlydepthstencil]
float4 main(PSInput input) : SV_Target2
{
	float4 Output = Diffuse.Sample(Default, input.uv);
	FEEDBACK_WRITE(g_feedback, Diffuse, Default, input.uv);
	return Output;
}