
#include "BaseTexture.h"

BaseTexture::~BaseTexture()
{}

void BaseTexture::CreateTextureFromData(void * data, int type, int width, int height, int bits)
{
	TextureDescription Tex = TextureDescription::DefaultTexture(width, height);
	Tex.BitDepth = bits;
	Tex.PtrToData = data;
	CreateTextureFromDesc(Tex);
}

ETextureType::Type BaseTexture::GetType()
{
	return CurrentTextureType;
}

DeviceContext * BaseTexture::GetContext()const
{
	return Context;
}

TextureDescription TextureDescription::DefaultTexture(int width, int height)
{
	TextureDescription Tex;
	Tex.BitDepth = 4;
	Tex.Width = width;
	Tex.Height = height;
	return Tex;
}

TextureDescription TextureDescription::DefaultTextTexture(int width, int height)
{
	TextureDescription Tex = DefaultTexture(width, height);
	Tex.MipLevels = 1;
	Tex.BitDepth = 1;
	Tex.Format = eTEXTURE_FORMAT::FORMAT_R8_UNORM;
	return Tex;
}
