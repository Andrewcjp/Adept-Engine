#include "VRX\VRRCommon.hlsl"
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUTPUT main(float4 position : POSITION)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = position;
	const float2 fliped = float2 (position.x, -position.y);
	output.uv = (fliped + float2(1, 1)) / 2.0f;
#if 0	
	if (position.y > 0.1f)
	{
		output.VRRIndex = 0;
		output.VRRtIndex = 0;
	}
	else
	{
		output.VRRIndex = 1;
		output.VRRtIndex = 1;
	}
	//VRR_PROCESS(output);
#endif
	return output;
}
