struct GSOutput
{
	float4 Pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float3 LightPos: TEXCOORD0;
	float3 WorldPos:NORMAL0;
};
float main(GSOutput input) : SV_Depth
{
#if DIRECTIONAL
	return 1.0f;
#else
	float Farplane = 500;
	float LightDistance = length(input.WorldPos.xyz - input.LightPos);
	LightDistance = LightDistance / Farplane;
	return LightDistance;
#endif
}

