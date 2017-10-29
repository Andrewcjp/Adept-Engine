#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "ImageIO.h"

EImageIOStatus ImageIO::LoadUncompressedTGA(unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels, FILE* pf)
{
	unsigned char header[6];

	if(fread(header, sizeof(header), 1, pf) == 0)
	{
		return E_IMAGEIO_ERROR;
	}
	
	*sizeX = ((int)header[1]<<8) | header[0];
	*sizeY = ((int)header[3]<<8) | header[2];
	*bpp = header[4];
	
	if( (*sizeX <= 0) || (*sizeY <= 0) || ((*bpp != 24) && (*bpp != 32)))
	{
		return E_IMAGEIO_ERROR;
	}

	//NOW WE ARE PRETTY SURE the file contains proper image data.
	//Allocate some memory for the buffer
	*nChannels = (*bpp)>>3;
	int dataSize = (*sizeX)*(*sizeY)*(*nChannels);
	*buffer = (unsigned char*)malloc(dataSize);

	if(buffer == NULL)
	{
		return E_IMAGEIO_ERROR;
	}

	if(fread(*buffer, 1, dataSize, pf) != dataSize)
	{
		delete [] (*buffer);
		*buffer = NULL;
		return E_IMAGEIO_ERROR;
	}
	
	for(int cswap = 0; cswap < dataSize; cswap += (*nChannels))
	{
		(*buffer)[cswap] ^= (*buffer)[cswap+2]^=(*buffer)[cswap] ^= (*buffer)[cswap+2];
	}

	return E_IMAGEIO_SUCCESS;
}

EImageIOStatus ImageIO::LoadTGA(const char* filename, unsigned char** buffer, int* sizeX, int* sizeY, int* bpp, int* nChannels)
{
	FILE* pfile = NULL;
	EImageIOStatus result = E_IMAGEIO_SUCCESS;
	errno_t err = 0;
	unsigned char header[12];
	unsigned char UncompressedTGASigniture[12] = {0,0,2,0,0,0,0,0,0,0,0,0}; 
	unsigned char CompressedTGASigniture[12] = {0,0,10,0,0,0,0,0,0,0,0,0}; 

	*buffer = NULL;

	err = fopen_s(&pfile, filename, "rb");

	if (err)
	{
		printf("Error opening image file: %s\n", filename);
		return E_IMAGEIO_ERROR;
	}

	if(!pfile)
	{
		return E_IMAGEIO_FILENOTFOUND;
	}

	if(fread(header, sizeof(header), 1, pfile) == 0)
	{
		return E_IMAGEIO_ERROR;
	}
	
	if(memcmp(UncompressedTGASigniture, header, sizeof(header))==0)
	{
		//TODO: load uncompressed tga
		result = LoadUncompressedTGA(buffer, sizeX, sizeY, bpp, nChannels, pfile);
	}
	else if(memcmp(UncompressedTGASigniture, header, sizeof(header))==0)
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
