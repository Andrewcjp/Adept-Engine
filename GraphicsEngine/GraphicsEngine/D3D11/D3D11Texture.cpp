#include "EngineGlobals.h"
#if BUILD_D3D11
#include <windows.h>
#include "../RHI/RHI.h"
#include "../Core/Assets/ImageIO.h"
#include "D3D11Texture.h"
#include "../Core/Assets/ImageLoader.h"
#include "../Core/Engine.h"
D3D11Texture::D3D11Texture()
{
	m_textureRV = NULL;
}

D3D11Texture::~D3D11Texture()
{
	FreeTexture();
}

D3D11Texture::D3D11Texture(const char * path, bool tga)
{
	this->tga = tga;
	std::string rpath = Engine::GetRootDir();
	rpath.append("\\asset\\texture\\");
	rpath.append(path);
	CreateTextureFromFile(RHI::GetD3DDevice(), rpath.c_str());
}

void D3D11Texture::CreateTextureFromFile(ID3D11Device* , const char* filename)
{
	//N.B. Limitation here, the default implementation here only loads texture in R8G8B8A8 format 
	//make sure your image file also contains an alpha channel.
	if (m_textureRV != NULL)
	{
		return;
	}

	unsigned char *buffer = NULL;
	int width;
	int height;
	int bpp = 0;
	int nChannels;
	if (tga)
	{
		if (ImageIO::LoadTGA(filename, &buffer, &width, &height, &bpp, &nChannels) != E_IMAGEIO_SUCCESS)
		{
			return;
		}
	}
	else
	{
		buffer = ImageLoader::instance->LoadSOILFile(&width, &height, &nChannels, filename);
		
	}

	if (buffer == NULL)//this shouldn't happen, but let's do it anyway.
	{
		return;
	}

	CreateTextureFromData(buffer, 0, width, height, nChannels);

	free(buffer);
}

void D3D11Texture::CreateTextureFromData(void* data, int , int width, int height, int nChannels)
{
	D3D11_SUBRESOURCE_DATA initdata;
	ZeroMemory(&initdata, sizeof(initdata));
	nChannels = 4;//soil returns the non padded channel count
	initdata.SysMemPitch = sizeof(unsigned char)*nChannels*width;


	initdata.pSysMem = data;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	/*if (nChannels == 3)
	{
		desc.Format = DXGI_FORMAT_BC1_UNORM_SRGB;
	}
	else if (nChannels == 4)
	{*/
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	}

	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Texture2D* pTex = NULL;

	HRESULT hr = RHI::GetD3DDevice()->CreateTexture2D(&desc, &initdata, &pTex);

	if (SUCCEEDED(hr) && pTex)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		ZeroMemory(&srvd, sizeof(srvd));

		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MipLevels = 1;

		hr = RHI::GetD3DDevice()->CreateShaderResourceView(pTex, &srvd, &m_textureRV);

		if (FAILED(hr))
		{
			pTex->Release();
		}

		if (m_textureRV == NULL)
		{
			pTex->Release();
		}
	}
}

void D3D11Texture::CreateTextureAsRenderTarget(int , int )
{

}

void D3D11Texture::CreateTextureAsDepthTarget(int , int )
{

}


void D3D11Texture::FreeTexture()
{
	if (m_textureRV != NULL)
	{
		m_textureRV->Release();
	}
}
void D3D11Texture::Bind(int unit)
{
	RHI::GetD3DContext()->PSSetSamplers(unit, 1, &RHI::instance->m_texSamplerDefaultState);
	RHI::GetD3DContext()->PSSetShaderResources(unit, 1, &m_textureRV);
}
void D3D11Texture::SetTextureID(int )
{
}
void D3D11Texture::CreateTextureAsRenderTarget(int , int , bool , bool )
{
}
#endif