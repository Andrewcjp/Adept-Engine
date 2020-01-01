#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
RWTexture2D<uint> RateData : register(u0);
PUSHCONST cbuffer ResData : register(b1)
{
	int2 Resolution;
};
Texture2D<float4> GBuffer_Pos : register(t0);
Texture2D<float4> ShadowMask : register(t1);
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int ShadingRate = SHADING_RATE_1X1;
	bool Hit = false;
	int2 Origin = DTid.xy*VRS_TILE_SIZE;
	for (int x = 0; x < VRS_TILE_SIZE; x++)
	{
		for (int y = 0; y < VRS_TILE_SIZE; y++)
		{
			int2 samplepos = Origin + int2(x, y);
			if (samplepos.x >= Resolution.x || samplepos.y >= Resolution.y)
			{
				continue;
			}
			float z = ShadowMask[samplepos].x;
			if (z == 0)
			{
				Hit = true;
				break;
			}
		}
	}
	if (!Hit)
	{
		ShadingRate = SHADING_RATE_2X2;
	}
	RateData[DTid.xy] = ShadingRate;
}

