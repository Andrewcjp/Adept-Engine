#pragma once
#include "RHI/RHICommandList.h"

class DXDescriptor;
class D3D12FrameBuffer;
class D3D12RHITexture;
class D3D12DeviceContext;
class RHITexture;
struct RHITextureDesc2;
class DescriptorHeap;
class FrameBuffer;

class D3D12RHITextureArray : public RHITextureArray
{
public:
	//#RHI: Ensure Framebuffer srv matches!
	D3D12RHITextureArray(DeviceContext* device, int inNumEntries);
	virtual ~D3D12RHITextureArray();
	RHI_VIRTUAL void AddFrameBufferBind(FrameBuffer* Buffer, int slot)override;
	RHI_VIRTUAL void BindToShader(RHICommandList* list, int slot)override;
	RHI_VIRTUAL void SetIndexNull(int TargetIndex, FrameBuffer* Buffer = nullptr);
	RHI_VIRTUAL void SetFrameBufferFormat(const RHIFrameBufferDesc & desc);
	DXDescriptor* GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap = nullptr);
	uint64 GetHash();

	RHI_VIRTUAL void AddTexture(RHITexture* Tex, int Index) override;
	RHI_VIRTUAL void SetTextureFormat(const RHITextureDesc2& Desc) override;

private:
	void Release() override;
	void Clear() override;
	std::vector<D3D12FrameBuffer*> LinkedBuffers;
	std::vector<D3D12RHITexture*> LinkedTextures;
	D3D12_SHADER_RESOURCE_VIEW_DESC NullHeapDesc = {};
	D3D12DeviceContext* Device = nullptr;
};

