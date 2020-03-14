#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
RWTexture2D<uint> RateData : register(u0);
cbuffer GData : register(b1)
{
	float GeoThreshold;
	int FullResTheshold;
	int HalfResTheshold;
	bool DebugEdgeCount;
	bool WriteZeroImage;
};
PUSHCONST cbuffer ResData : register(b2)
{
	int2 Resolution;
};
Texture2D<float4> GBuffer_Pos : register(t0);
Texture2D<float4> ShadowMask : register(t1);
Texture2D<float4> GBuffer_Tex : register(t2);
float avg_intensity(in float3 pix)
{
	return (pix.r + pix.g + pix.b) / 3.;
}
float threshold(in float thr1, in float thr2, in float val)
{
	if (val < thr1) { return 0.0; }
	if (val > thr2) { return 1.0; }
	return val;
}
float3 get_pixel(in uint2 coords, in int dx, in int dy)
{
	return GBuffer_Tex[coords + uint2(dx, dy)];
}
float IsEdge(uint2 coords)
{
	float pix[9];
	int k = -1;
	float delta;

	// read neighboring pixel intensities
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			k++;
			pix[k] = avg_intensity(get_pixel(coords, i,j));
		}
	}

	// average color differences around neighboring pixels
	delta = (abs(pix[1] - pix[7]) +
		abs(pix[5] - pix[3]) +
		abs(pix[0] - pix[8]) +
		abs(pix[2] - pix[6])
		) / 4.;

	return threshold(GeoThreshold, 1.0, clamp(delta, 0.0, 1.0));
}
bool DetectEdgeAtPX(uint2 px)
{	
	return (IsEdge(px) >  0.0);
}
#define SHADOWONLY 0
uint DetmineRate(uint2 Origin)
{
	bool SampleHit = false;
	int EdgesHit = 0;
	for (int x = 0; x < VRS_TILE_SIZE; x += 1)
	{
		for (int y = 0; y < VRS_TILE_SIZE; y += 1)
		{
			int2 samplepos = Origin + int2(x, y);
			if (samplepos.x >= Resolution.x || samplepos.y >= Resolution.y)
			{
				continue;
			}
#if SHADOWONLY
			float z = ShadowMask[samplepos].x;	
			if (z == 0)
			{
				return SHADING_RATE_1X1;
			}
#endif
			if (GBuffer_Tex[samplepos].r > GeoThreshold)
			{
				SampleHit = true;
				EdgesHit++;
			}
		
		}
	}
	if (DebugEdgeCount)
	{
		return EdgesHit;
	}
#if SHADOWONLY
	return SHADING_RATE_2X2;
#endif
	if (EdgesHit >= FullResTheshold)
	{
		return SHADING_RATE_1X1;
	}
	if (EdgesHit >= HalfResTheshold)
	{
		return SHADING_RATE_2X2;
	}
	return SHADING_RATE_4X4;
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint2 Origin = DTid.xy*VRS_TILE_SIZE;
	if (WriteZeroImage)
	{
		RateData[DTid.xy] = SHADING_RATE_1X1;
	}
	else
	{
		RateData[DTid.xy] = SHADING_RATE_2X2;// DetmineRate(Origin);
	}
}

