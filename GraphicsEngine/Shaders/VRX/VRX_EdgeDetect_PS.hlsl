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
PUSHCONST cbuffer ResData : register(b2)
{
	int2 Resolution;
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
Texture2D<float4> GBuffer_Normal;
Texture2D<float>  GBuffer_Depth;
SamplerState defaultSampler : register (s0);

float GetDepth(float Sample)
{
	const float near = 0.1f;
	const float far = 10.0f;
	float Z = 2.0 * Sample - 1.0;
	float lineardepth = (2.0f * near) / (far + near - Z * (far - near));
	return lineardepth;
}

float3 GetColourAtPos(uint2 xy)
{
	float3 Normal = GBuffer_Normal[xy].xyz;
	float Depth = GetDepth(GBuffer_Depth[xy]);
	float3 EdgeColor = Normal/* + Depth*/;
	return EdgeColor;
}
float2 offsets[8] = {
					float2(-1, -1),
					float2(-1, 0),
					float2(-1, 1),
					float2(0, -1),
					float2(0, 1),
					float2(1, -1),
					float2(1, 0),
					float2(1, 1)
};
float main(PSInput input) : SV_TARGET
{
	
	int2 PixelPos = input.uv * Resolution;
	float3 RegionColour = float3(0, 0, 0);
	for (int j = 0; j < 8; j++)
	{
		RegionColour += GetColourAtPos(PixelPos + offsets[j]);
	}
	RegionColour /= 8;
	float edgeStrength = length(RegionColour - GetColourAtPos(PixelPos));
	return edgeStrength;
}


