#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 12
#define FXAA_GREEN_AS_LUMA 1
#include "FXAA_SRC.hlsl"
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
PUSHCONST cbuffer Res: register(b10)
{
	int2 Res;
}
//todo: move this post tone map and pass in lumia 
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 invRes = 1.0 / Res.xy;
	float2 ScreenPos = float2((float)DTid.x / (float)Res.x, (float)DTid.y / (float)Res.y);
	FxaaTex InputFXAATex = { InputSampler, InputTexture };
	float4 output =  FxaaPixelShader(
		ScreenPos,							// FxaaFloat2 pos,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsolePosPos,
		InputFXAATex,							// FxaaTex tex,
		InputFXAATex,							// FxaaTex fxaaConsole360TexExpBiasNegOne,
		InputFXAATex,							// FxaaTex fxaaConsole360TexExpBiasNegTwo,
		invRes, //RCPFrame.xy	// FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f,									// FxaaFloat fxaaQualitySubpix,
		0.166f,									// FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f,								// FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f,									// FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)		// FxaaFloat fxaaConsole360ConstDir,
	);
	DstTexture[DTid.xy] = output;
}

