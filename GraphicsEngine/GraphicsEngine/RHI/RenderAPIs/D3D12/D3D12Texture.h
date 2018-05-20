#pragma once
#include "../RHI/BaseTexture.h"
#include <d3d12.h>
#include <vector>

#include "../EngineGlobals.h"
namespace nv_dds
{
	class CDDSImage;
}
class D3D12Texture :
	public BaseTexture
{
public:

	D3D12Texture(class DeviceContext * inDevice = nullptr);
	unsigned char * GenerateMip(int & startwidth, int & startheight, int bpp, unsigned char * StartData, int & mipsize, float ratio = 2.0f);
	
	unsigned char * GenerateMips(int count, int StartWidth, int StartHeight, unsigned char * startdata);
	bool CLoad(std::string name);

	bool LoadDDS(std::string filename);

	//D3D12Texture(std::string name,class DeviceContext * Device);
	virtual ~D3D12Texture();
	bool CreateFromFile(std::string FileName) override;
	virtual void Bind(int unit) {};
	void Bind(CommandListDef * list) override;
	void BindToSlot(CommandListDef * list, int slot);
	virtual void FreeTexture() {};
	virtual void SetTextureID(int id) {} ;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) {} ;
	virtual void CreateTextureFromData(void * data, int type, int width, int height, int bits) override;
	void UpdateSRV();
	static float MipCreationTime;
	ID3D12Resource* GetResource()
	{
		return m_texture;
	}
	int width = 0;
	int height = 0;
	UINT16 Miplevels = 6;
	int	MipLevelsReadyNow = 1;
	bool CheckDevice(int index);
private:
	int TextureWidth = 100;
	int TextureHeight = 100;
	D3D12_SUBRESOURCE_DATA Texturedatarray[9];
	static const UINT TexturePixelSize = 4;

	class DeviceContext * Device = nullptr;
	DXGI_FORMAT format;
	ID3D12Resource* m_texture = nullptr;
	class DescriptorHeap* srvHeap = nullptr;
	// Inherited via BaseTexture
	
};

