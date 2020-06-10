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

#if HARDWARE_SUPPORT_SAMPLER_FEEDBACK
#define FEEDBACKTEXTURE(Type) FeedbackTexture2D<Type>
#define FEEDBACK_WRITE(FeedBack,TextureTarget,Sampler,UV) FeedBack.WriteSamplerFeedback(TextureTarget, Sampler, UV);

#define SFS_TYPE_MIPSUSED SAMPLER_FEEDBACK_MIP_REGION_USED
#define SFS_TYPE_MINMIP SAMPLER_FEEDBACK_MIN_MIP

#else
#define FEEDBACKTEXTURE(Type) RWTexture2D<float> 
#define FEEDBACK_BIND Texture2D SFS_MipTexture:register(t100);
#define FEEDBACK_WRITE(FeedBack,TextureTarget,Sampler,UV) float mip = SFS_MipTexture.Sample(Sampler,UV).r; uint W, H = 0; FeedBack.GetDimensions(W, H); FeedBack[UV*uint2(W, H)] = min(FeedBack[UV*uint2(W, H)],mip);
//check width max on this?
#define SFS_TYPE_MIPSUSED 0
#define SFS_TYPE_MINMIP 0
#endif

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