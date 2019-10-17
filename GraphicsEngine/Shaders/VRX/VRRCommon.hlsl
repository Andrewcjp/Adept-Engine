#if SUPPORT_VRR
#define VRS_TILE_SIZE 16
int2 GetShadingRate(int ID)
{
	if (ID == 2)
	{
		return int2(1, 2);
		return int2(2, 2);
	}
	else if (ID == 3)
	{
		return int2(2, 2);
	}
	return int2(1, 1);
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
	int2 MaxPoint = GetTileID((1.0 - size) * res);
	int2 min = GetTileID(size * res);

	return IsWithinRectTile(GetTileID(ScreenUV*res), min, MaxPoint);
}
int GetShadingRateIDForPixel(float2 ScreenUV, int2 Res)
{
	float Size = 0.2;
	if (IsWithinRect(ScreenUV, Res, 0.25))
	{
		return 1;
	}
	//if (IsWithinRect(ScreenUV, Res, 0.35))
	//{
	//	return 2;
	//}
	return 2;
}
bool ShouldShadePixel(float2 ScreenUV, int2 res)
{
	int Rate = GetShadingRateIDForPixel(ScreenUV, res);
	int2 PixelPos = ScreenUV * res;
	if (Rate == 1)
	{
		return true;
	}
	return IsPixelSource(PixelPos, GetShadingRate(Rate));
}

#define VRS_INPUTBIND Texture2D<uint> VRSTexture : register( t66 );
#else
#define VRS_INPUTBIND
#endif