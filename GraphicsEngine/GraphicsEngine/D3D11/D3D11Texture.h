
#include "EngineGlobals.h"
#if BUILD_D3D11
#ifndef __TEXTURED3D11_H__
#define __TEXTURED3D11_H__

#include <d3d11_1.h>

#include "RHI/BaseTexture.h"

class D3D11Texture : public BaseTexture
{
public:
	ID3D11ShaderResourceView*		m_textureRV;
	bool tga = false;
private:
	
public:
	D3D11Texture();
	virtual							~D3D11Texture();
	D3D11Texture(const char* path, bool tga);

	virtual void					CreateTextureFromFile(ID3D11Device* pDevice, const char* filename);
	virtual void					CreateTextureFromData(void* data, int type, int width, int height, int bits)override;

	virtual void					CreateTextureAsRenderTarget(int width, int height);

	virtual void					CreateTextureAsDepthTarget(int width, int height);
	virtual void					FreeTexture();

	// Inherited via BaseTexture
	virtual void Bind(int unit) override;
	virtual void SetTextureID(int id) override;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) override;
};

#endif
#endif