TextureCube<float4> Cubemap : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
#include "Lighting.hlsl"
cbuffer CB : register(b0)
{
	float Roughness;
	int Mip;
	int2 Threads;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int x = max(DTid.x, 1);
	int y = max(DTid.y, 1);
	int2 T = int2(128, 128);
	float3 localPos = float3((float)x / T.x, (float)y / T.y, 1);

	//localPos.z = 0;

	float roughness = Roughness;
	float3 N = normalize(localPos);
	float3 R = N;
	float3 V = R;

	const uint SAMPLE_COUNT = 128u;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0, 0, 0);
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			prefilteredColor += Cubemap.SampleLevel(BilinearClamp, localPos, 0).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;
	prefilteredColor = Cubemap.SampleLevel(BilinearClamp, N, 0).rgb;
	//prefilteredColor = localPos;
	//Write the final color into the destination texture.
	DstTexture[DTid.xy] = float4(prefilteredColor, 1.0f);
}