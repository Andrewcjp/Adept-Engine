#ifndef __IMAGEIO_H__
#define __IMAGEIO_H__

#include <stdio.h>

enum EImageIOStatus
{
	E_IMAGEIO_ERROR = 0,
	E_IMAGEIO_FILENOTFOUND,
	E_IMAGEIO_SUCCESS
};

class ImageIO
{
	private:
		static EImageIOStatus LoadUncompressedTGA(unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels, FILE* pf); 
	public:
		static EImageIOStatus LoadTGA(const char* filename, unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels);
};

#endif
