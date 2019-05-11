#include "ImageIO.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "RHI/BaseTexture.h"
#include "RHI/RHI.h"
#include "Core/EngineInc.h"
#include "Core/Engine.h"
#include "Core/Utils/FileUtils.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/Core/Defaults.h"
//#Asset Move all texture loading here and add support of more formats
//#Assets create common texture format holder? texture asset?
ImageIO* ImageIO::instance = nullptr;
ImageIO::ImageIO()
{

}

ImageIO::~ImageIO()
{
	for (int i = 0; i < LoadedTextures.size(); i++)
	{
		SafeRefRelease(LoadedTextures[i]);
	}
}

BaseTexture * ImageIO::GetDefaultTexture()
{
	return Defaults::GetDefaultTexture().Get();
}

void ImageIO::StartLoader()
{
	if (instance == nullptr)
	{
		instance = new ImageIO();
	}
}

void ImageIO::ShutDown()
{
	if (instance)
	{
		SafeDelete(instance);
	}
}

void ImageIO::RegisterTextureLoad(BaseTexture* newtex)
{
	if (instance && newtex)
	{
		instance->LoadedTextures.push_back(newtex);
	}
}

bool ImageIO::CheckIfLoaded(std::string name, BaseTexture ** out)
{
	if (instance)
	{
		return instance->IN_CheckIfLoaded(name, out);
	}
	return false;
}

bool ImageIO::IN_CheckIfLoaded(std::string name, BaseTexture ** out)
{
	for (unsigned int i = 0; i < LoadedTextures.size(); i++)
	{
		if (LoadedTextures[i]->TexturePath == name)
		{
			if (LoadedTextures[i]->IsPendingKill())
			{
				LoadedTextures.erase(LoadedTextures.begin() + i);
				return false;
			}
			*out = LoadedTextures[i];
			return true;
		}
	}
	return false;
}


EImageIOStatus ImageIO::LoadTexture2D(const char* filename, unsigned char** buffer, int* width, int* height, int* nchan)
{
	*buffer = stbi_load(filename, width, height, nchan, STBI_rgb_alpha);
	if (*buffer == nullptr)
	{
		Log::OutS << "Failed to Load Texture '" << filename << "'\n";
		return EImageIOStatus::E_IMAGEIO_ERROR;
	}
	return EImageIOStatus::E_IMAGEIO_SUCCESS;
}

EImageIOStatus ImageIO::LoadTextureCubeMap(const char* filename, unsigned char** buffer, int* width, int* height)
{
	//#Asset Cube map loading 
	//Stub!
	return EImageIOStatus::E_IMAGEIO_ERROR;
}