#pragma once
#include "RHI/BaseTexture.h"
#include "DXDescriptor.h"
class DeviceContext;
class DescriptorHeap;
class D3D12DeviceContext;
class D3D12CommandList;
class DXDescriptor;
class GPUResource;
class D3D12RHITexture;
class D3D12Texture : public BaseTexture
{
public:
	D3D12Texture(DeviceContext * inDevice = nullptr);
	virtual ~D3D12Texture();
	void BindToSlot(D3D12CommandList * list, int slot);
	virtual void CreateTextureFromDesc(const TextureDescription& desc) override;
	virtual void CreateAsNull() override;
	GPUResource* GetResource();
	int Width = 0;
	int Height = 0;
	UINT16 Miplevels = 6;
	int	MipLevelsReadyNow = 1;
	bool CheckDevice(int index);
	DXDescriptor* GetDescriptor(RHIViewDesc Desc, DescriptorHeap* heap = nullptr);
	DescriptorItemDesc GetItemDesc(const RHIViewDesc& Desc)const;
protected:
	void Release() override;
private:
	D3D12_SUBRESOURCE_DATA Texturedatarray[9];
	D3D12DeviceContext * Device = nullptr;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//#TextureLoading Remove this hack
	bool UsingDDSLoad = false;
	int FrameCreated = -1;
	D3D12RHITexture* Cover = nullptr;
};

