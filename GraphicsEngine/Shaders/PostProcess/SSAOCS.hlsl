
Texture2D<float4> PosTex : register(t0);
Texture2D<float4> NormalTex : register(t1);
Texture2D<float4> NoiseTex: register(t2);
Texture2D<float4> DepthTexture : register(t3);

RWTexture2D<float4> DstTexture : register(u0);
SamplerState DefaultSampler : register(s0);


cbuffer Data: register(b0)
{
	float radius;
	float bias;
	int kernelSize;
	float4x4 projection;
	float4x4 view;
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
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{

	const int Width = 1920 / 4;
	const int height = 1080 / 4;

	float3 pos = PosTex[DTid.xy].xyz;//world space
	pos = ConvertPos(pos);
	float3 Normal = normalize(NormalTex[DTid.xy].xyz);
	float3 RandomDir = normalize(samples[DTid.y % 4]);
	//RandomDir = float3(0, 1, 0);
	Normal = Convertzero(Normal);//world to view space
	RandomDir = Convertzero(RandomDir);//world to view space
	//Normal = normalize(Normal);
	//RandomDir = normalize(RandomDir);
	float3 tangent = normalize(RandomDir - Normal * dot(RandomDir, Normal));
	float3 bitangent = cross(Normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, Normal);

	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; ++i)
	{
		// get sample position
		float3 tsample = mul(samples[i], TBN); // from tangent to view-space
		tsample = pos + tsample * radius;
		// project sample position (to sample texture) (to get position on screen/texture)
		float4 offset = float4(tsample, 1.0);
		offset = mul(offset, projection); // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

		float sampleDepth = PosTex[offset.xy].z; // get depth value of kernel sample
		sampleDepth = ConvertPos(PosTex[offset.xy]).z;
		// range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pos.z - sampleDepth));
		occlusion += (sampleDepth >= tsample.z + bias ? 1.0 : 0.0) * rangeCheck;
		//occlusion = pos.z;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
//	occlusion = Normal.x;
	DstTexture[DTid.xy] = float4(occlusion, occlusion, occlusion, 1.0f);
	//DstTexture[DTid.xy] = float4(Normal, 1.0f);
}
