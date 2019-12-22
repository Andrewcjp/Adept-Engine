#if SUPPORT_VRR
#include "VRX/VRXCommon.hlsl"
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
int2 GetTileID(int2 PixelPos)
{
	int2 Tile = PixelPos / int2(VRS_TILE_SIZE, VRS_TILE_SIZE);
	return Tile;
}
bool IsPixelSource(int2 PixelPos, int2 CPixelSize)
{
	if ((PixelPos.x % CPixelSize.x == 0) && (PixelPos.y % CPixelSize.y == 0))
	{
		return true;
	}
	return false;
}
bool IsWithinRectTile(int2 tile, int2 Size, int2 end)
{
	if (tile.x > Size.x && tile.x < end.x)
	{
		if (tile.y > Size.y && tile.y < end.y)
		{
			return true;
		}
	}
	return false;
}
bool IsWithinRect(float2 ScreenUV, int2 res, float size)
{
	int2 MaxPoint = ((1.0 - size) * res);
	int2 min = (size * res);

	return IsWithinRectTile(ScreenUV*res, min, MaxPoint);
}
int GetShadingRateIDForPixel(float2 ScreenUV, int2 Res)
{
	float Size = 0.2;
	if (IsWithinRect(ScreenUV, Res, 0.35))
	{
		return D3D12_MAKE_COARSE_SHADING_RATE(RATE_1X, RATE_1X);
	}
	if (IsWithinRect(ScreenUV, Res, 0.25))
	{
		return D3D12_MAKE_COARSE_SHADING_RATE(RATE_2X, RATE_2X);
	}
	return D3D12_MAKE_COARSE_SHADING_RATE(RATE_4X, RATE_4X);
}
bool ShouldShadePixel(float2 ScreenUV, int2 res)
{
	int Rate = GetShadingRateIDForPixel(ScreenUV, res);
	int2 PixelPos = ScreenUV * res;
	if (Rate == D3D12_MAKE_COARSE_SHADING_RATE(RATE_1X, RATE_1X))
	{
		return true;
	}
	return IsPixelSource(PixelPos, GetShadingRate(Rate));
}

#define VRS_INPUTBIND Texture2D<uint> VRSTexture : register( t66 );
#else
#define VRS_INPUTBIND
#endif