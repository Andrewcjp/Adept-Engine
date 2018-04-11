#pragma once
#include "../RHI/BaseTexture.h"
#include <d3d12.h>
#include <vector>

#include "../EngineGlobals.h"
class D3D12Texture :
	public BaseTexture
{
public:

	D3D12Texture(class DeviceContext * inDevice = nullptr);
	unsigned char * GenerateMip(int & startwidth, int & startheight, int bpp, unsigned char * StartData, int & mipsize, float ratio = 2.0f);
	
	unsigned char * GenerateMips(int count, int StartWidth, int StartHeight, unsigned char * startdata);
	void CLoad(std::string name);

	D3D12Texture(std::string name,class DeviceContext * Device);
	~D3D12Texture();
	void CreateTexture();
	virtual void Bind(int unit) override;
	void Bind(CommandListDef * list) override;
	void BindToSlot(CommandListDef * list, int slot);
	virtual void FreeTexture() override;
	virtual void SetTextureID(int id) override;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) override;
	virtual void CreateTextureFromData(void * data, int type, int width, int height, int bits) override;
	void UpdateSRV();
	ID3D12DescriptorHeap* m_srvHeap;
	static float MipCreationTime;
	ID3D12Resource* GetResource()
	{
		return m_texture;
	}
	int width = 0;
	int height = 0;
	UINT16 Miplevels = 6;
	int	MipLevelsReadyNow = 1;
private:
	int TextureWidth = 100;
	int TextureHeight = 100;
	D3D12_SUBRESOURCE_DATA Texturedatarray[9];
	static const UINT TexturePixelSize = 4;
	UINT8* GenerateCheckerBoardTextureData();
	class DeviceContext * Device = nullptr;
	DXGI_FORMAT format;
	ID3D12Resource* m_texture;
	std::string TextureName;
	// Inherited via BaseTexture
	
};

