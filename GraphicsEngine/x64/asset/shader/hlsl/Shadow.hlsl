#define SHADOW_DEPTH_BIAS 0.005f
loat4 CalcUnshadowedAmountPCF2x2(int lightid, float4 vPosWorld)
{
	// Compute pixel position in light space.
	float4 vLightSpacePos = vPosWorld;
	vLightSpacePos = mul(vLightSpacePos, lights[lightid].LightVP);

	vLightSpacePos.xyz /= vLightSpacePos.w;

	// Translate from homogeneous coords to texture coords.
	float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
	vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;

	// Depth bias to avoid pixel self-shadowing.
	float vLightSpaceDepth = vLightSpacePos.z - SHADOW_DEPTH_BIAS;

	// Find sub-pixel weights.//todo: shader define!
	float2 vShadowMapDims = float2(1024, 1024); // need to keep in sync with .cpp file
	float size = 1.0f;
	float4 vSubPixelCoords = float4(size, size, size, size);
	vSubPixelCoords.xy = frac(vShadowMapDims * vShadowTexCoord);
	vSubPixelCoords.zw = 1.0f - vSubPixelCoords.xy;
	float4 vBilinearWeights = vSubPixelCoords.zxzx * vSubPixelCoords.wwyy;

	// 2x2 percentage closer filtering.
	float2 vTexelUnits = 1.0f / vShadowMapDims;
	float4 vShadowDepths;
	if (lightid == 0)
	{
		vShadowDepths.x = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord);
		vShadowDepths.y = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f));
		vShadowDepths.z = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y));
		vShadowDepths.w = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits);
	}
	else
	{
		/*vShadowDepths.x = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord);
		vShadowDepths.y = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f));
		vShadowDepths.z = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y));
		vShadowDepths.w = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits);*/
	}
	// What weighted fraction of the 4 samples are nearer to the light than this pixel?
	float4 vShadowTests = (vShadowDepths >= vLightSpaceDepth) ? 1.0f : 0.0f;
	return dot(vBilinearWeights, vShadowTests);
}