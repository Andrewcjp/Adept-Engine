#pragma once

#include <vector>
#include <SOIL.h>
class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();
#if BUILD_OPENGL
	static void StartLoader() {
		if (instance == NULL) {
			instance = new ImageLoader();
		}
	}

	static ImageLoader* instance;
	int loadsplitCubeMap(std::string path);
	unsigned char * LoadSOILFile(int * width, int * height, int * nchan, const char * file);

	int LoadImageFile(std::string path);
	bool CheckIfLoaded(std::string name, int * out);
	/*void ImageLoader::AddToLoaded(OGLTexture* tex) {
		loadedtextures.push_back(tex);
	}*/
#endif
private:
	//std::vector<OGLTexture*> loadedtextures;
	
};

