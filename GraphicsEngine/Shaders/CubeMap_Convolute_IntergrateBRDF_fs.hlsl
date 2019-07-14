TextureCube texColour : register(t0);
SamplerState defaultSampler : register (s0);

cbuffer Data : register(b0)
{
	float3 Normal: Normal0;
	uint slice : SV_RenderTargetArrayIndex;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
#include "Lighting.hlsl"



float2 IntegrateBRDF(float NdotV, float roughness)
{
	float3 V;
	V.x = sqrt(1.0 - NdotV * NdotV);
	V.y = 0.0;
	V.z = NdotV;

	float A = 0.0;
	float B = 0.0;

	float3 N = float3(0.0, 0.0, 1.0);

	const uint SAMPLE_COUNT = 1024u;
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(L.z, 0.0);
		float NdotH = max(H.z, 0.0);
		float VdotH = max(dot(V, H), 0.0);

		if (NdotL > 0.0)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return float2(A, B);
}


float4 main(VS_OUTPUT input) : SV_Target
{
	float2 integratedBRDF = IntegrateBRDF(input.uv.x, input.uv.y);
	return float4(integratedBRDF.xy, 0.0f, 1.0f);
}
