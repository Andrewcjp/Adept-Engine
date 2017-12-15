
#include "OGLTexture.h"
#include "Core/Assets/ImageIO.h"
#include "Core/Assets/ImageLoader.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "glm\glm.hpp"
#include "../Core/Engine.h"
OGLTexture::OGLTexture()
{
	m_syshandle = m_apphandle = (unsigned int)-1;
}

OGLTexture::OGLTexture(const char* path, bool istga)
{
	Name = path;
	AssetName = Name.c_str();
	GLuint out;
	if (ImageLoader::instance->CheckIfLoaded(path, &out))
	{
		m_syshandle = out;
		return;
	}

	if (istga)
	{
		std::string rpath = Engine::GetRootDir();
		rpath.append("\\asset\\texture\\");
		rpath.append(path);
		CreateTextureFromFile(rpath.c_str());
	}
	else
	{
		CreateFromFile(path);
	}
	ImageLoader::instance->AddToLoaded(this);


}

OGLTexture::~OGLTexture()
{
	FreeTexture();
}

void OGLTexture::CreateFromFile(const char* path)
{
	m_syshandle = ImageLoader::instance->LoadImageFile(path);
}
void OGLTexture::CreateTextureFromFile(const char* filename)
{
	unsigned char *buffer = NULL;
	int width;
	int height;
	int bpp;
	int nChannels;

	if (ImageIO::LoadTGA(filename, &buffer, &width, &height, &bpp, &nChannels) != E_IMAGEIO_SUCCESS)
	{
		printf("Failed to load %s", filename);
		return;
	}

	if (buffer == NULL)//this shouldn't happen, but let's do it anyway.
	{
		return;
	}
	CreateTextureFromData(buffer, 0, width, height, nChannels);

	free(buffer);
}

void OGLTexture::CreateTextureFromData(void* data, int type, int width, int height, int nChannels)
{
	glGenTextures(1, &m_syshandle);
	glBindTexture(GL_TEXTURE_2D, m_syshandle);
	
	if (type == 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else if (type == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	if (nChannels == 1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	else if (nChannels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else if (nChannels == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
}

void OGLTexture::CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha)
{
	if (m_syshandle != -1)
	{
		//prevent losing texture data
		glDeleteTextures(1, &m_syshandle);
	}
	glGenTextures(1, &m_syshandle);
	glBindTexture(GL_TEXTURE_2D, m_syshandle);
	
	if (depthonly)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	else
	{
		if (alpha)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		}
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}

void OGLTexture::GenerateNoiseTex(float )
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoNoise;
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		glm::normalize(noise);
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &m_syshandle);
	glBindTexture(GL_TEXTURE_2D, m_syshandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void OGLTexture::CreateTextureAsRenderTargetOfType(int width, int height, GLenum type, GLenum perscion, GLenum InternalType)
{
	glGenTextures(1, &m_syshandle);
	glBindTexture(GL_TEXTURE_2D, m_syshandle);
	glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, InternalType, perscion, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}
void OGLTexture::CreateRenderCubeMap(int width, int height)
{
	glGenTextures(1, (&m_syshandle));
	IsCubeMap = true;
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_syshandle);
	for (GLuint i = 0; i < CUBE_SIDES; ++i)
	{//no more magic number
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
void OGLTexture::Bind(int unit)
{
	//glActiveTexture(GL_TEXTURE0 +unit);
	//glBindTexture(GL_TEXTURE_2D, m_syshandle);
	if (GPUStateCache::CheckStateOfUnit(unit, GetTextureHandle()) == false)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, GetTextureHandle());
		GPUStateCache::UpdateUnitState(unit, GetTextureHandle());
	}


}
void OGLTexture::SetTextureID(int id)
{
	//the texture has been genrated elseware overide
	m_syshandle = id;
}
void OGLTexture::CreateTextureAsDepthTarget(int width, int height)
{
	glGenTextures(1, &m_syshandle);
	glBindTexture(GL_TEXTURE_2D, m_syshandle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}


void OGLTexture::FreeTexture()
{
	if (m_syshandle != -1)
	{
		
		glDeleteTextures(1, &m_syshandle);
		m_syshandle = (unsigned int)-1;
		m_apphandle = (unsigned int)-1;
	}
}
