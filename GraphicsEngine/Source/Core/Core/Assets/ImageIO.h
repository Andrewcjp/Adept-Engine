#pragma once
#include "RHI\BaseTexture.h"

enum EImageIOStatus
{
	E_IMAGEIO_ERROR = 0,
	E_IMAGEIO_FILENOTFOUND,
	E_IMAGEIO_SUCCESS
};

class ImageIO
{
public:
	static BaseTextureRef GetDefaultTexture();
	static void StartLoader();
	static void ShutDown();
	static void RegisterTextureLoad(BaseTextureRef newtex);
	static bool CheckIfLoaded(std::string name, BaseTextureRef * out);
	CORE_API static EImageIOStatus LoadTexture2D(const char * filename, unsigned char ** buffer, int * width, int * height, int * nchan);
	CORE_API static EImageIOStatus LoadTextureCubeMap(const char * filename, unsigned char ** buffer, int * width, int * height);

private:
	ImageIO();
	~ImageIO();
	static ImageIO* instance;
	std::vector<BaseTextureRef> LoadedTextures;
	bool IN_CheckIfLoaded(std::string name, BaseTextureRef * out);
};

