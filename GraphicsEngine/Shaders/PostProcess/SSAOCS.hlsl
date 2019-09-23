
Texture2D<float4> PosTex : register(t0);
Texture2D<float4> NormalTex : register(t1);
Texture2D<float4> NoiseTex: register(t2);
Texture2D<float4> DepthTexture : register(t3);

RWTexture2D<float4> DstTexture : register(u0);
SamplerState DefaultSampler : register(s0);


cbuffer Data: register(b0)
{
	row_major float4x4 projection;
	row_major float4x4 view;
	float radius;
	float bias;
	int kernelSize;
	float3 samples[64];
};
float3 ConvertPos(float3 pos)
{
	//return pos;
	return mul(float4(pos, 1.0f), view).xyz;//view space
}
float3 Convertzero(float3 pos)
{
	//return pos;
	return mul(float4(pos, 0.0f), view).xyz;//view space
}

float3 ConvertViewToNDC(float3 ViewSpace)
{	
	float4 ClipSpace = mul(ViewSpace, projection);
	float3 ndcSpacePos = ClipSpace.xyz / ClipSpace.w;
	ndcSpacePos.xyz = ndcSpacePos.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
	ndcSpacePos.y = -ndcSpacePos.y;
	return ndcSpacePos;
}

float3 ConvertWorldToNDC(float3 pos)
{
	float4 ViewSpace = mul(float4(pos, 1.0f), view);
	return ConvertViewToNDC(ViewSpace);
}
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
#if 1
	const int Width = 1920 / 4;
	const int height = 1080 / 4;

	float3 pos = PosTex[DTid.xy].xyz;//world space
	float3 Normal = normalize(NormalTex[DTid.xy].xyz);
	float3 RandomDir = normalize(samples[DTid.x% 32]);
#define VS 1
#if VS
	pos = ConvertPos(pos);
	Normal = Convertzero(Normal);
	RandomDir = Convertzero(RandomDir);
#endif
	float3 tangent = normalize(RandomDir - Normal * dot(RandomDir, Normal));
	float3 bitangent = cross(Normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, Normal);

	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		float3 tsample = mul(samples[i], TBN); // from tangent to view-space
		//tsample = float3(0, 1, 0);
		tsample = pos + tsample * radius;
		// project sample position (to sample texture) (to get position on screen/texture)
#if VS
		float3 ndcSpacePos = ConvertViewToNDC(tsample);
#else
		float3 ndcSpacePos = ConvertWorldToNDC(tsample);
#endif
		float sampleDepth = PosTex.SampleLevel(DefaultSampler, ndcSpacePos.xy, 0).z;

		// range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - sampleDepth));
		occlusion += (sampleDepth >= tsample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);

	DstTexture[DTid.xy] = float4(occlusion, occlusion, occlusion, 1.0f);

#else
	float3 pos = PosTex[DTid.xy].xyz;//world space
	pos = ConvertPos(pos);
	float3 Normal = normalize(NormalTex[DTid.xy].xyz);
	float3 RandomDir = normalize(samples[DTid.x % 32]);
	//Normal = Convertzero(Normal);
	float3 tangent = normalize(RandomDir - Normal * dot(RandomDir, Normal));
	float3 bitangent = cross(Normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, Normal);
	float3 tsample = mul(samples[0], TBN); // from tangent to view-space

	tsample = pos + tsample * radius;

	float3 ndcSpacePos = ConvertViewToNDC(tsample);

	float posW = PosTex.SampleLevel(DefaultSampler, ndcSpacePos.xy, 0);
	float sampleDepth = ConvertPos(posW).z;

	//float3 Normal = normalize(NormalTex[DTid.xy].xyz);
	//
	//float3 ndcSpacePos = ConvertToNDC(pos);

	//float3 PosScreen  = PosTex.SampleLevel(DefaultSampler,ndcSpacePos.xy,0).xyz;//world space
	DstTexture[DTid.xy] = float4(sampleDepth, sampleDepth, sampleDepth, 1.0f);
#endif
}
