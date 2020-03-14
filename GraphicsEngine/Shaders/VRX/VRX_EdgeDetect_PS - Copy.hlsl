cbuffer GData : register(b1)
{
	float GeoThreshold;
	int FullResTheshold;
	int HalfResTheshold;
};
struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
float FwidthFine(float4 dif)
{
	return abs(ddx_fine(dif)) + abs(ddy_fine(dif));
}

float sigmoid(float a, float f)
{
	return 1.0 / (1.0 + exp(-f * a));
}
Texture2D<float4> GBuffer_Diffuse;
SamplerState defaultSampler : register (s0);
float main(PSInput input) : SV_TARGET
{
	float4 dif = GBuffer_Diffuse.Sample(defaultSampler,input.uv);

	float edgeStrength = length(FwidthFine(dif));
	/*if (edgeStrength > 0.01)
	{
		edgeStrength = 1.0f;
	}
	else
	{
		edgeStrength = 0.0f;
	}*/
	edgeStrength = sigmoid(edgeStrength - 0.15, 32.0);
	return edgeStrength;
}


