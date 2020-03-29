#include "D3D12InterGPUStagingResource.h"
#include "D3D12RHI.h"
#include "RHI\RHICommandList.h"
#include "RHI\RHIInterGPUStagingResource.h"
#include "RHI\RHI.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"

D3D12InterGPUStagingResource::D3D12InterGPUStagingResource(DeviceContext* owner, const InterGPUDesc& desc) :RHIInterGPUStagingResource(owner, desc)
{
	Init();
}


D3D12InterGPUStagingResource::~D3D12InterGPUStagingResource()
{

}

GPUResource * D3D12InterGPUStagingResource::GetViewOnDevice(int index)
{
	return GPUViews[index].SharedResource;
}

void D3D12InterGPUStagingResource::Init()
{
	ID3D12Device* Host = D3D12RHI::DXConv(OwnerDevice)->GetDevice();
	UINT64 textureSize = 0;
	if (Desc.IsBuffer)
	{
		textureSize = Desc.BufferDesc.ElementCount * Desc.BufferDesc.Stride;
	}
	else
	{

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
		UINT64 pTotalBytes = 0;
		DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(Desc.FramebufferDesc.RTFormats[0]);
		CD3DX12_RESOURCE_DESC renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, Desc.FramebufferDesc.Width, Desc.FramebufferDesc.Height
			, Desc.FramebufferDesc.TextureDepth, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
		Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &layout, nullptr, nullptr, &pTotalBytes);
		textureSize = D3D12Helpers::Align(layout.Footprint.RowPitch * layout.Footprint.Height);
	}

	CrossAdapterDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);

	CD3DX12_HEAP_DESC heapDesc(
		textureSize,
		D3D12_HEAP_TYPE_DEFAULT,
		0,
		D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);
	//heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	//heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;//l1?
	if (MainHeap != nullptr)
	{
		SafeRelease(MainHeap);
	}
	Host->CreateHeap(&heapDesc, ID_PASS(&MainHeap));

	ThrowIfFailed(Host->CreateSharedHandle(
		MainHeap,
		nullptr,
		GENERIC_ALL,
		nullptr,
		&heapHandle));

	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (Desc.Mask.GetFlagValue(i))
		{
			CreateForGPU(i);
		}
	}
}

void D3D12InterGPUStagingResource::CreateForGPU(int index)
{
	if (RHI::GetDeviceContext(index) == nullptr)
	{
		return;
	}
	ID3D12Device* Target = D3D12RHI::DXConv(RHI::GetDeviceContext(index))->GetDevice();
	HRESULT openSharedHandleResult = Target->OpenSharedHandle(heapHandle, ID_PASS(&GPUViews[index].SharedHeap));
	ensure(openSharedHandleResult == S_OK);
	ID3D12Resource* Res = nullptr;
	ThrowIfFailed(Target->CreatePlacedResource(
		GPUViews[index].SharedHeap,
		0,
		&CrossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		nullptr,
		ID_PASS(&Res)));

	GPUViews[index].SharedResource = new GPUResource(Res, D3D12_RESOURCE_STATE_COPY_SOURCE, RHI::GetDeviceContext(index));
}
