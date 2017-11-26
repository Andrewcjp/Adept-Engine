#pragma once
#include "../RHI/BaseTexture.h"
#include <d3d12.h>
#include <vector>
class D3D12Texture :
	public BaseTexture
{
public:
	D3D12Texture();
	~D3D12Texture();
	void CreateTexture();
	virtual void Bind(int unit) override;
	void Bind(ID3D12GraphicsCommandList * list);
	virtual void FreeTexture() override;
	virtual void SetTextureID(int id) override;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) override;
	virtual void CreateTextureFromData(void * data, int type, int width, int height, int bits) override;
	ID3D12DescriptorHeap* m_srvHeap;
private:
	int TextureWidth = 100;
	int TextureHeight = 100;

	static const UINT TexturePixelSize = 4;
	UINT8* GenerateCheckerBoardTextureData();

	ID3D12Resource* m_texture;

	// Inherited via BaseTexture
	
};

