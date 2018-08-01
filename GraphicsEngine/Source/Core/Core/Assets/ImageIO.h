#ifndef __IMAGEIO_H__
#define __IMAGEIO_H__

#include <stdio.h>
#include <vector>
enum EImageIOStatus
{
	E_IMAGEIO_ERROR = 0,
	E_IMAGEIO_FILENOTFOUND,
	E_IMAGEIO_SUCCESS
};
namespace nv_dds
{
	class CDDSImage;
}
class ImageIO
{
	ImageIO();
	~ImageIO();
	static ImageIO* instance;
	class BaseTexture* DefaultTexture = nullptr;
	static EImageIOStatus LoadUncompressedTGA(unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels, FILE* pf);
	std::vector<class BaseTexture*> LoadedTextures;
	bool IN_CheckIfLoaded(std::string name, BaseTexture ** out);
public:
	static BaseTexture* GetDefaultTexture();
	static void StartLoader();
	static void ShutDown();
	static void RegisterTextureLoad(BaseTexture * newtex);
	static bool CheckIfLoaded(std::string name, BaseTexture ** out);
	__declspec(dllexport) static EImageIOStatus LoadTGA(const char* filename, unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels);
	__declspec(dllexport) static EImageIOStatus LoadTexture2D(const char * filename, unsigned char ** buffer, int * width, int * height, int * nchan);
	__declspec(dllexport) static EImageIOStatus LoadTextureCubeMap(const char * filename, unsigned char ** buffer, int * width, int * height);

};

#endif
