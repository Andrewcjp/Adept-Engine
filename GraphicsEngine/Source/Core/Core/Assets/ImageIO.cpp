#include "ImageIO.h"
#include <SOIL.h>
#include "RHI/BaseTexture.h"
#include "RHI/RHI.h"
#include "Core/EngineInc.h"
#include "Core/Engine.h"
#include "Core/Utils/FileUtils.h"
#include "Core/Platform/PlatformCore.h"
ImageIO* ImageIO::instance = nullptr;
ImageIO::ImageIO()
{
	DefaultTexture = AssetManager::DirectLoadTextureAsset("\\texture\\T_GridSmall_01_D.png");
	ensureFatalMsgf(DefaultTexture, "Failed to Load Fallback Texture");
	DefaultTexture->AddRef();
}

ImageIO::~ImageIO()
{
	SafeRefRelease(DefaultTexture);
}

EImageIOStatus ImageIO::LoadUncompressedTGA(unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels, FILE* pf)
{
	unsigned char header[6];

	if (fread(header, sizeof(header), 1, pf) == 0)
	{
		return E_IMAGEIO_ERROR;
	}

	*sizeX = ((int)header[1] << 8) | header[0];
	*sizeY = ((int)header[3] << 8) | header[2];
	*bpp = header[4];

	if ((*sizeX <= 0) || (*sizeY <= 0) || ((*bpp != 24) && (*bpp != 32)))
	{
		return E_IMAGEIO_ERROR;
	}

	//NOW WE ARE PRETTY SURE the file contains proper image data.
	//Allocate some memory for the buffer
	*nChannels = (*bpp) >> 3;
	int dataSize = (*sizeX)*(*sizeY)*(*nChannels);
	*buffer = (unsigned char*)malloc(dataSize);

	if (buffer == NULL)
	{
		return E_IMAGEIO_ERROR;
	}

	if (fread(*buffer, 1, dataSize, pf) != dataSize)
	{
		delete[](*buffer);
		*buffer = NULL;
		return E_IMAGEIO_ERROR;
	}

	for (int cswap = 0; cswap < dataSize; cswap += (*nChannels))
	{
		(*buffer)[cswap] ^= (*buffer)[cswap + 2] ^= (*buffer)[cswap] ^= (*buffer)[cswap + 2];
	}

	return E_IMAGEIO_SUCCESS;
}

BaseTexture * ImageIO::GetDefaultTexture()
{
	if (instance)
	{
		return instance->DefaultTexture;
	}
	return nullptr;
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
		delete instance;
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
			*out = (LoadedTextures[i]);
			return true;
		}
	}
	return false;
}

EImageIOStatus ImageIO::LoadTGA(const char* filename, unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels)
{
	FILE* pfile = NULL;
	EImageIOStatus result = E_IMAGEIO_SUCCESS;
	errno_t err = 0;
	unsigned char header[12];
	unsigned char UncompressedTGASigniture[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
	unsigned char CompressedTGASigniture[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };

	*buffer = NULL;

	err = fopen_s(&pfile, filename, "rb");

	if (err)
	{
		printf("Error opening image file: %s\n", filename);
		return E_IMAGEIO_ERROR;
	}

	if (!pfile)
	{
		return E_IMAGEIO_FILENOTFOUND;
	}

	if (fread(header, sizeof(header), 1, pfile) == 0)
	{
		return E_IMAGEIO_ERROR;
	}

	if (memcmp(UncompressedTGASigniture, header, sizeof(header)) == 0)
	{
		//TODO: load uncompressed tga
		result = LoadUncompressedTGA(buffer, sizeX, sizeY, bpp, nChannels, pfile);
	}
	else if (memcmp(UncompressedTGASigniture, header, sizeof(header)) == 0)
	{
		//TODO: load compressed tga
	}
	else
	{
		//unrecognised header signiture.
		fclose(pfile);
		return E_IMAGEIO_ERROR;
	}

	fclose(pfile);

	return result;
}
EImageIOStatus ImageIO::LoadTexture2D(const char* filename, unsigned char** buffer, int* width, int* height, int* nchan)
{
	*buffer = SOIL_load_image(filename, width, height, nchan, SOIL_LOAD_RGBA);
	if (*buffer == nullptr)
	{
		Log::OutS << "Failed to Load Texture '" << filename << "'\n";
		return EImageIOStatus::E_IMAGEIO_ERROR;
	}
	return EImageIOStatus::E_IMAGEIO_SUCCESS;
}

EImageIOStatus ImageIO::LoadTextureCubeMap(const char* filename, unsigned char** buffer, int* width, int* height)
{
	//todo: this
	//Stub!
	return EImageIOStatus::E_IMAGEIO_ERROR;
}