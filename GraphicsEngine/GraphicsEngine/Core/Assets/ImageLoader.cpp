#include "ImageLoader.h"
#include "Core/Assets/AssetManager.h"


ImageLoader::ImageLoader()
{

}


ImageLoader::~ImageLoader()
{
}
#if BUILD_OPENGL
int ImageLoader::loadsplitCubeMap(std::string path)
{ImageLoader* ImageLoader::instance = NULL;
	std::vector<const GLchar*> faces;
	faces.push_back("right.jpg");
	faces.push_back("left.jpg");
	faces.push_back("top.jpg");
	faces.push_back("bottom.jpg");
	faces.push_back("back.jpg");
	faces.push_back("front.jpg");

	int textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (int i = 0; i < faces.size(); i++)
	{
		std::string facepath = AssetManager::instance->TextureAssetPath;
		facepath.append(faces[i]);
		image = SOIL_load_image(facepath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
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
int ImageLoader::LoadImageFile(std::string path)
{
	int textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	TextureAsset Image;
	if (!AssetManager::instance->GetTextureAsset(path, Image))
	{
		//__debugbreak();
		return (int)-1;
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

	float MAxAn;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MAxAn);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MAxAn);
	glBindTexture(GL_TEXTURE_2D, 0);
	//delete[] image;

	return textureID;
}
//
//bool ImageLoader::CheckIfLoaded(std::string name, int * out)
//{
//	for (unsigned int i = 0; i < loadedtextures.size(); i++)
//	{
//		if (loadedtextures[i]->Name == name)
//		{
//			*out = (loadedtextures[i]->m_syshandle);
//			return true;
//		}
//	}
//	return false;
//}
//
#endif