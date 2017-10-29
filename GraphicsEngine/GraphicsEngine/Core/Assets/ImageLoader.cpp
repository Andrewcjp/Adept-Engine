#include "ImageLoader.h"
#include "Core/Assets/AssetManager.h"

ImageLoader* ImageLoader::instance = NULL;
ImageLoader::ImageLoader()
{

}


ImageLoader::~ImageLoader()
{
}

GLuint ImageLoader::loadsplitCubeMap(std::string path)
{
	std::vector<const GLchar*> faces;

	faces.push_back("../asset/texture/right.jpg");
	faces.push_back("../asset/texture/left.jpg");
	faces.push_back("../asset/texture/top.jpg");
	faces.push_back("../asset/texture/bottom.jpg");
	faces.push_back("../asset/texture/back.jpg");
	faces.push_back("../asset/texture/front.jpg");

	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
		);
		delete[] image;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;

}
unsigned char* ImageLoader::LoadSOILFile(int *width, int *height, int *nchan, const char* file)
{
	//R8G8B8A8
	unsigned char* image = SOIL_load_image(file, width, height, nchan, SOIL_LOAD_RGBA);
	if (image == nullptr)
	{
		printf("Load texture Error %s\n", file);
	}
	return image;
}
GLuint ImageLoader::LoadImageFile(std::string path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	//int width, height;
//	unsigned char* image;

	glBindTexture(GL_TEXTURE_2D, textureID);

	////image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
	//if (image == nullptr)
	//{
	//	printf("Load texture Error %s\n", path.c_str());
	//	return -1;
	//}
	TextureAsset Image;
	if (!AssetManager::instance->GetTextureAsset(path, Image))
	{
		return -1;
	}
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA8, Image.Width, Image.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image.image
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	GLfloat MAxAn;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MAxAn);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MAxAn);
	glBindTexture(GL_TEXTURE_2D, 0);
	//delete[] image;

	return textureID;
}

bool ImageLoader::CheckIfLoaded(std::string name, GLuint * out)
{
	for (unsigned int i = 0; i < loadedtextures.size(); i++)
	{
		if (loadedtextures[i]->Name == name)
		{
			*out = (loadedtextures[i]->m_syshandle);
			return true;
		}
	}
	return false;
}

