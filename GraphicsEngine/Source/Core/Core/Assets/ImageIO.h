#pragma once

enum EImageIOStatus
{
	E_IMAGEIO_ERROR = 0,
	E_IMAGEIO_FILENOTFOUND,
	E_IMAGEIO_SUCCESS
};

class ImageIO
{
public:
	static BaseTexture* GetDefaultTexture();
	static void StartLoader();
	static void ShutDown();
	static void RegisterTextureLoad(BaseTexture * newtex);
	static bool CheckIfLoaded(std::string name, BaseTexture ** out);
	CORE_API static EImageIOStatus LoadTexture2D(const char * filename, unsigned char ** buffer, int * width, int * height, int * nchan);
	CORE_API static EImageIOStatus LoadTextureCubeMap(const char * filename, unsigned char ** buffer, int * width, int * height);

private:
	ImageIO();
	~ImageIO();
	static ImageIO* instance;
	class BaseTexture* DefaultTexture = nullptr;
	std::vector<class BaseTexture*> LoadedTextures;
	bool IN_CheckIfLoaded(std::string name, BaseTexture ** out);
};

