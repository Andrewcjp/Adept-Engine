#ifndef __TEXTUREGL_H__
#define __TEXTUREGL_H__
#define CUBE_SIDES 6
#include "RHI/BaseTexture.h"
#include <string>
#include <GLEW\GL\glew.h>
#include <vector>
#include <random>
#include "glm\fwd.hpp"
class OGLTexture : public BaseTexture
{
private:

public:
	unsigned int					m_syshandle;
	unsigned int					m_apphandle;
	std::string						Name;

public:
	OGLTexture();

	OGLTexture(const char * path, bool istga = false);
	virtual							~OGLTexture();

	void CreateFromFile(const char * path);

	virtual void					CreateTextureFromFile(const char* filename);
	virtual void					CreateTextureFromData(void* data, int type, int width, int height, int bits)override;

	virtual void					CreateTextureAsRenderTarget(int width, int height, bool depthonly = false, bool alpha = true) override;
	void GenerateNoiseTex(float noiseSize = 4);
	void CreateTextureAsRenderTargetOfType(int width, int height, GLenum type = GL_RGB, GLenum perscion = GL_UNSIGNED_BYTE, GLenum InternalType = GL_RGB);
	void CreateRenderCubeMap(int width, int height);
	void Bind(int unit)override;

	void SetTextureID(int id);

	virtual void					CreateTextureAsDepthTarget(int width, int height);
	virtual void					FreeTexture() override;
	int GetTextureHandle() {
		return m_syshandle;
	}
	bool IsCubeMap = false;
};

#endif
