#define SHADOW_DEPTH_BIAS 0.005f


float4 CalcUnshadowedAmountPCF2x2(Light LightObj, float4 vPosWorld, Texture2D ShadowTex)
{
#if 0
	// Compute pixel position in light space.
	float4 vLightSpacePos = vPosWorld;
	vLightSpacePos = mul(vLightSpacePos, LightObj.LightVP);

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
	float4 vShadowDepths = float4(0, 0, 0, 0);
	if (LightObj.ShadowID == 0)
	{
		vShadowDepths.x = ShadowTex.Sample(g_Clampsampler, vShadowTexCoord).r;
		vShadowDepths.y = ShadowTex.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f)).r;
		vShadowDepths.z = ShadowTex.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y)).r;
		vShadowDepths.w = ShadowTex.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits).r;
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
	return dot(vBilinearWeights, vShadowTests);6
#else
	return 0.0f;
#endif
}

float ShadowCalculationCube(const float3 fragPos, Light lpos, TextureCube ShadowTex)
{
	// Get vector between fragment position and light position
	float3 fragToLight = (fragPos - lpos.LPosition);
	float currentDepth = length(fragToLight);
	float bias = 0.5f;
	float far_plane = 500;
	float closestDepth = 0;
	int id = lpos.ShadowID;

	closestDepth = ShadowTex.Sample(g_Clampsampler, fragToLight).r;
	closestDepth *= far_plane;
	float output = 0.0f;

	if (currentDepth - bias > closestDepth)
	{
		output = 1.0f;
	}
	return output;
}

float GetShadowPreSampled(float2 UV, Texture2D Map)
{
	return 1.0f;
}