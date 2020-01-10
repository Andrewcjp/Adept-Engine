#if SUPPORT_VRR
#include "VRX/VRXCommon.hlsl"
static const uint TILE_HEADER_OFFSET_NOP = 0;
static const uint TILE_HEADER_OFFSET_VARTILES = 4;
int2 GetShadingRate(int ID)
{	
	int2 decoded = int2(D3D12_GET_COARSE_SHADING_RATE_X_AXIS(ID), D3D12_GET_COARSE_SHADING_RATE_Y_AXIS(ID));
	const int RateArray[] = {
		1,
		2,
		4,
	};
	return int2(RateArray[decoded.x], RateArray[decoded.y]);
}
bool IsPixelSource(uint2 PixelPos, uint2 CPixelSize)
{
	if ((PixelPos.x % CPixelSize.x == 0) && (PixelPos.y % CPixelSize.y == 0))
	{
		return true;
	}
	return false;
}
#if SHADER_SUPPORT_VRR
Texture2D<uint> VRSTexture : register( t66 );
bool ShouldShadePixel(float2 ScreenUV, int2 res)
{
	int2 PixelPos = ScreenUV * res;
	int Rate = VRSTexture[PixelPos.xy/ VRS_TILE_SIZE];
	if (Rate == D3D12_MAKE_COARSE_SHADING_RATE(RATE_1X, RATE_1X))
	{
		return true;
	}
	return IsPixelSource(PixelPos, GetShadingRate(Rate));
}
#endif
#else
#define VRS_INPUTBIND
#endif