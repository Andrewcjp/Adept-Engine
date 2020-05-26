#include "BaseTexture.h"
#pragma warning(push,0)
#undef max
#include "gli/gli.hpp"
#include "RHITexture.h"
#pragma warning(pop)
BaseTexture::~BaseTexture()
{}

bool BaseTexture::CreateFromFile(AssetPathRef FileName)
{
	std::string FilePAth = FileName.GetFullPathToAsset();
	gli::texture tex = gli::load(FilePAth);
	if (tex.empty())
	{
		return false;
	}
	Log::LogMessage("Loading texture " + FilePAth);
	Description.Width = tex.extent().x;
	Description.Height = tex.extent().y;
	Description.MipLevels = tex.levels();
	Description.BitDepth = 4;// texChannels;

	//todo: handle this better!
	Description.PtrToData = malloc(tex.size());
	memcpy(Description.PtrToData, tex.data(), tex.size());
	Description.Faces = tex.faces();
	Description.ImageByteSize = tex.size();
	for (int i = 0; i < tex.levels(); i++)
	{
		Description.MipLevelExtents.push_back(glm::ivec2(tex.extent(i).x, tex.extent(i).y));
	}
	if (tex.target() == gli::TARGET_CUBE)
	{
		Description.TextureType = ETextureType::Type_CubeMap;
		CurrentTextureType = ETextureType::Type_CubeMap;
	}
	CreateTextureFromDesc(Description);
	TexturePath = FileName.GetRelativePathToAsset();
	return true;
}
RHITexture* BaseTexture::CreateFromFile2(AssetPathRef FileName)
{
	std::string FilePAth = FileName.GetFullPathToAsset();
	gli::texture tex = gli::load(FilePAth);
	if (tex.empty())
	{
		return false;
	}
	RHITexture* RHITex = RHI::GetRHIClass()->CreateTexture2();
	Log::LogMessage("Loading texture " + FilePAth);
	TextureDescription Description;
	Description.Width = tex.extent().x;
	Description.Height = tex.extent().y;
	Description.MipLevels = tex.levels();
	Description.BitDepth = 4;// texChannels;

	//todo: handle this better!
	Description.PtrToData = malloc(tex.size());
	memcpy(Description.PtrToData, tex.data(), tex.size());
	Description.Faces = tex.faces();
	Description.ImageByteSize = tex.size();
	for (int i = 0; i < tex.levels(); i++)
	{
		Description.MipLevelExtents.push_back(glm::ivec2(tex.extent(i).x, tex.extent(i).y));
	}
	if (tex.target() == gli::TARGET_CUBE)
	{
		Description.TextureType = ETextureType::Type_CubeMap;
		//CurrentTextureType = ETextureType::Type_CubeMap;
	}
	RHITex->CreateWithUpload(Description, RHI::GetDefaultDevice());

	return RHITex;
}
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

uint64_t TextureDescription::Size(int mip)
{
	if (MipLevelExtents.size() == 0)
	{
		return Width * Height*BitDepth;
	}
	return MipLevelExtents[mip].x*MipLevelExtents[mip].y * BitDepth;
}

glm::ivec2 TextureDescription::MipExtents(int mip)
{
	if (MipLevelExtents.size() == 0)
	{
		return glm::ivec2(Width, Height);
	}
	return MipLevelExtents[mip];
}
