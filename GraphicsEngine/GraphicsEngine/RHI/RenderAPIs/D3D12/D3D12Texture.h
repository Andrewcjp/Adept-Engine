#pragma once
#include "RHI/BaseTexture.h"
#include <d3d12.h>
#include <vector>
#include "EngineGlobals.h"

class D3D12Texture : public BaseTexture
{
public:
	static float MipCreationTime;
	D3D12Texture(class DeviceContext * inDevice = nullptr);
	virtual ~D3D12Texture();
	bool CreateFromFile(AssetPathRef  FileName) override;
	virtual void Bind(int unit) {};
	void BindToSlot(ID3D12GraphicsCommandList * list, int slot);
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) {} ;
	virtual void CreateTextureFromData(void * data, int type, int width, int height, int bits) override;
	virtual void CreateAsNull() override;
	void UpdateSRV();	
	ID3D12Resource* GetResource();
	int width = 0;
	int height = 0;
	UINT16 Miplevels = 6;
	int	MipLevelsReadyNow = 1;
	bool CheckDevice(int index);
private:
	unsigned char * GenerateMip(int & startwidth, int & startheight, int bpp, unsigned char * StartData, int & mipsize, float ratio = 2.0f);
	unsigned char * GenerateMips(int count, int StartWidth, int StartHeight, unsigned char * startdata);
	bool CLoad(AssetPathRef name);
	bool LoadDDS(std::string filename);
	D3D12_SUBRESOURCE_DATA Texturedatarray[9];
	class DeviceContext * Device = nullptr;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ID3D12Resource* m_texture = nullptr;
	class DescriptorHeap* srvHeap = nullptr;	
};

