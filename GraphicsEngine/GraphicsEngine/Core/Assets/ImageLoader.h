#pragma once
#include <GLEW\GL\glew.h>
#include <vector>
#include <SOIL.h>
#include "OpenGL\OGLTexture.h"
class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();
	static void StartLoader() {
		if (instance == NULL) {
			instance = new ImageLoader();
		}
	}
	static ImageLoader* instance;
	GLuint loadsplitCubeMap(std::string path);
	unsigned char * LoadSOILFile(int * width, int * height, int * nchan, const char * file);

	GLuint LoadImageFile(std::string path);
	bool CheckIfLoaded(std::string name, GLuint * out);
	void ImageLoader::AddToLoaded(OGLTexture* tex) {
		loadedtextures.push_back(tex);
	}
private:
	std::vector<OGLTexture*> loadedtextures;
	
};

