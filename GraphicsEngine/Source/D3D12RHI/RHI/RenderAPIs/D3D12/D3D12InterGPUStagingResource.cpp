#include "D3D12InterGPUStagingResource.h"
#include "D3D12RHI.h"
#include "RHI\RHICommandList.h"
#include "RHI\RHIInterGPUStagingResource.h"
#include "RHI\RHI.h"


D3D12InterGPUStagingResource::D3D12InterGPUStagingResource(DeviceContext* owner, const InterGPUDesc& desc) :RHIInterGPUStagingResource(owner, desc)
{

}


D3D12InterGPUStagingResource::~D3D12InterGPUStagingResource()
{

}

ID3D12Resource * D3D12InterGPUStagingResource::GetViewOnDevice(int index)
{
	return GPUViews[index].SharedResource;
}

void D3D12InterGPUStagingResource::Init()
{
	ID3D12Device* Host = D3D12RHI::DXConv(OwnerDevice)->GetDevice();
#if 0
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	UINT64 pTotalBytes = 0;
	Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &layout, nullptr, nullptr, &pTotalBytes);
	UINT64 textureSize = D3D12Helpers::Align(layout.Footprint.RowPitch * layout.Footprint.Height);

	D3D12_RESOURCE_DESC crossAdapterDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);

	CD3DX12_HEAP_DESC heapDesc(
		textureSize,
		D3D12_HEAP_TYPE_DEFAULT,
		0,
		D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);
	//heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	//heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;//l1?
	if (CrossHeap != nullptr)
	{
		SafeRelease(CrossHeap);
	}
	Host->CreateHeap(&heapDesc, IID_PPV_ARGS(&CrossHeap));

	ThrowIfFailed(Host->CreateSharedHandle(
		CrossHeap,
		nullptr,
		GENERIC_ALL,
		nullptr,
		&heapHandle));

	HRESULT openSharedHandleResult = Target->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&TWO_CrossHeap));
	ensure(openSharedHandleResult == S_OK);
#endif
}

void D3D12InterGPUStagingResource::CreateForGPU(int index)
{
	ID3D12Device* Target = D3D12RHI::DXConv(RHI::GetDeviceContext(index))->GetDevice();
	HRESULT openSharedHandleResult = Target->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&GPUViews[index].SharedHeap));
	ensure(openSharedHandleResult == S_OK);
	ThrowIfFailed(Target->CreatePlacedResource(
		GPUViews[index].SharedHeap,
		0,
		&CrossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		nullptr,
		IID_PPV_ARGS(&GPUViews[index].SharedResource)));
}
