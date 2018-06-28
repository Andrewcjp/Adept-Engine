#include "stdafx.h"
#include "D3D12Framebuffer.h"
#include "D3D12RHI.h"
#include "GPUResource.h"
#include "RHI/DeviceContext.h"
#include <algorithm>
#include "DescriptorHeap.h"
#include "Core/Performance/PerfManager.h"
#define CUBE_SIDES 6

void D3D12FrameBuffer::CreateSRVHeap(int Num)
{
	if (SrvHeap != nullptr)
	{
		SrvHeap->Release();
		delete SrvHeap;
	}
	SrvHeap = new DescriptorHeap(CurrentDevice, Num, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SrvHeap->SetName(L"Framebuffer Heap");
	if (NullHeap == nullptr)
	{
		NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &GetSrvDesc(0), NullHeap->GetCPUAddress(0));
	}
}
void D3D12FrameBuffer::CreateSRVInHeap(int HeapOffset, DescriptorHeap* targetheap)
{
	CreateSRVInHeap(HeapOffset, targetheap, CurrentDevice);
}
void D3D12FrameBuffer::CreateSRVInHeap(int HeapOffset, DescriptorHeap* targetheap, DeviceContext* target)
{
	if (BufferDesc.RenderTargetCount > 2)
	{

		target->GetDevice()->CreateShaderResourceView(RenderTarget[HeapOffset]->GetResource(), &GetSrvDesc(HeapOffset), targetheap->GetCPUAddress(HeapOffset));
	}
	else
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(BufferDesc.Dimension);
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Texture2DArray.MipLevels = 1;
		shadowSrvDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (BufferDesc.RenderTargetCount == 0)
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.DepthReadFormat);
			target->GetDevice()->CreateShaderResourceView(DepthStencil->GetResource(), &GetSrvDesc(0), targetheap->GetCPUAddress(HeapOffset));
		}
		else
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
			target->GetDevice()->CreateShaderResourceView(RenderTarget[0]->GetResource(), &GetSrvDesc(0), targetheap->GetCPUAddress(HeapOffset));
		}
	}
}

D3D12_SHADER_RESOURCE_VIEW_DESC D3D12FrameBuffer::GetSrvDesc(int RenderTargetIndex)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	if (BufferDesc.RenderTargetCount > 2)
	{
		shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(BufferDesc.Dimension);
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[RenderTargetIndex]);
	}
	else
	{
		shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(BufferDesc.Dimension);
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Texture2DArray.MipLevels = 1;
		shadowSrvDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (BufferDesc.RenderTargetCount == 0)
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.DepthReadFormat);
		}
		else
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
		}
	}
	return shadowSrvDesc;
}


bool D3D12FrameBuffer::CheckDevice(int index)
{
	if (CurrentDevice != nullptr)
	{
		if (OtherDevice != nullptr)
		{
			return (OtherDevice->GetDeviceIndex() == index) || (CurrentDevice->GetDeviceIndex() == index);
		}
		return (CurrentDevice->GetDeviceIndex() == index);
	}
	return false;
}

void D3D12FrameBuffer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	CurrentDevice->CPUWaitForAll();
	if (OtherDevice != nullptr)
	{
		OtherDevice->CPUWaitForAll();
	}

	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RenderTarget[i]->Release();
	}
	if (DepthStencil)
	{
		DepthStencil->Release();
	}
	Init();
	if (OtherDevice != nullptr)
	{
		SetupCopyToDevice(OtherDevice);
	}
}

static inline UINT Align(UINT size, UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
{
	return (size + alignment - 1) & ~(alignment - 1);
}

void D3D12FrameBuffer::SetupCopyToDevice(DeviceContext * device)
{
	OtherDevice = device;
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();

	DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
	if (BufferDesc.RenderTargetCount == 0)
	{
		readFormat = DXGI_FORMAT_R32_FLOAT;
	}
	renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, m_width, m_height, BufferDesc.TextureDepth,1 , 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);


	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	UINT64 pTotalBytes = 0;
	Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &layout, nullptr, nullptr, &pTotalBytes);
	UINT64 textureSize = Align(layout.Footprint.RowPitch * layout.Footprint.Height);

	// Create a buffer with the same layout as the render target texture.
	D3D12_RESOURCE_DESC crossAdapterDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);

	CD3DX12_HEAP_DESC heapDesc(
		textureSize,
		D3D12_HEAP_TYPE_DEFAULT,
		0,
		D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);
	//heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	//heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;//l1?
	Host->CreateHeap(&heapDesc, IID_PPV_ARGS(&CrossHeap));

	ThrowIfFailed(Host->CreateSharedHandle(
		CrossHeap,
		nullptr,
		GENERIC_ALL,
		nullptr,
		&heapHandle));

	HRESULT openSharedHandleResult = Target->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&TWO_CrossHeap));

	// We can close the handle after opening the cross-adapter shared resource.
	CloseHandle(heapHandle);
	//target
	ThrowIfFailed(Host->CreatePlacedResource(
		CrossHeap,
		0,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&PrimaryRes)));
	//source
	ThrowIfFailed(Target->CreatePlacedResource(
		TWO_CrossHeap,
		0,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		nullptr,
		IID_PPV_ARGS(&Stagedres)));

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = readFormat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;
	//renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
	ThrowIfFailed(Target->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&renderTargetDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&FinalOut)
	));

	SharedSRVHeap = new DescriptorHeap(OtherDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SharedTarget = new GPUResource(FinalOut, D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = readFormat;
	SrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(BufferDesc.Dimension);
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Texture2D.PlaneSlice = 0;
	SrvDesc.TextureCube.MipLevels = 1;

	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	OtherDevice->GetDevice()->CreateShaderResourceView(FinalOut, &SrvDesc, SharedSRVHeap->GetCPUAddress(0));
}

void D3D12FrameBuffer::TransitionTOCopy(ID3D12GraphicsCommandList* list)
{
	RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_COMMON);
}

void D3D12FrameBuffer::CopyToDevice(ID3D12GraphicsCommandList* list)
{
	PerfManager::StartTimer("CopyToDevice");
	// Copy the intermediate render target into the shared buffer using the
	// memory layout prescribed by the render target.
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;
	GPUResource* TargetResource = RenderTarget[0];
	if (BufferDesc.RenderTargetCount == 0)
	{
		TargetResource = DepthStencil;
	}
	TargetResource->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_SOURCE);

	CD3DX12_BOX box(0, 0, m_width, m_height);
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION dest(PrimaryRes, renderTargetLayout);
		CD3DX12_TEXTURE_COPY_LOCATION src(TargetResource->GetResource(), i);
		list->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
	PerfManager::EndTimer("CopyToDevice");
}

void D3D12FrameBuffer::MakeReadyOnTarget(ID3D12GraphicsCommandList* list)
{
	PerfManager::StartTimer("MakeReadyOnTarget");
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();
	// Copy the buffer in the shared heap into a texture that the secondary
	// adapter can sample from.	
	// Copy the shared buffer contents into the texture using the memory
	// layout prescribed by the texture.
	D3D12_RESOURCE_DESC secondaryAdapterTexture = FinalOut->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;
	SharedTarget->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(FinalOut, offset);
		Host->GetCopyableFootprints(&secondaryAdapterTexture, offset, 1, 0, &textureLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION src(Stagedres, textureLayout);
		CD3DX12_BOX box(0, 0, m_width, m_height);

		list->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
	PerfManager::EndTimer("MakeReadyOnTarget");
}

void D3D12FrameBuffer::MakeReadyForRead(ID3D12GraphicsCommandList * list)
{
	SharedTarget->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void D3D12FrameBuffer::MakeReadyForCopy(ID3D12GraphicsCommandList * list)
{	
	SharedTarget->SetResourceState(list, D3D12_RESOURCE_STATE_COMMON);//D3D12_RESOURCE_STATE_COPY_DEST
}

void D3D12FrameBuffer::BindDepthWithColourPassthrough(ID3D12GraphicsCommandList * list, D3D12FrameBuffer * Passtrhough)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
	if (Passtrhough->RenderTarget[0])
	{
		Passtrhough->RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	list->OMSetRenderTargets(Passtrhough->BufferDesc.RenderTargetCount, &Passtrhough->RTVHeap->GetCPUAddress(0), true, &DSVHeap->GetCPUAddress(0));
}

D3D12FrameBuffer::D3D12FrameBuffer(DeviceContext * device, RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{
	Init();
	if (BufferDesc.IsShared)
	{
		SetupCopyToDevice(BufferDesc.DeviceToCopyTo);
	}
}

void D3D12FrameBuffer::UpdateSRV()
{
	if (SrvHeap == nullptr)
	{
		SrvHeap = new DescriptorHeap(CurrentDevice, std::max(BufferDesc.RenderTargetCount, 1), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		SrvHeap->SetName(L"FrameBuffer SRV Heap");
	}
	if (NullHeap == nullptr)
	{
		NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	if (BufferDesc.RenderTargetCount == 0)
	{
		CreateSRVInHeap(0, SrvHeap);
	}
	else
	{
		for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
		{
			CreateSRVInHeap(i, SrvHeap);
		}
	}
}

void D3D12FrameBuffer::CreateResource(GPUResource** Resourceptr, DescriptorHeap* Heapptr, bool IsDepthStencil, DXGI_FORMAT Format, eTextureDimension ViewDimension, int OffsetInHeap)
{
	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = Format;
	if (IsDepthStencil)
	{
		ClearValue.DepthStencil.Depth = BufferDesc.DepthClearValue;
		ClearValue.DepthStencil.Stencil = 0;
	}
	else
	{
		ClearValue.Color[0] = BufferDesc.clearcolour.r;
		ClearValue.Color[1] = BufferDesc.clearcolour.g;
		ClearValue.Color[2] = BufferDesc.clearcolour.b;
		ClearValue.Color[3] = BufferDesc.clearcolour.a;
	}
	ID3D12Resource* NewResource = nullptr;
	CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC();
	ResourceDesc.Width = m_width;
	ResourceDesc.Height = m_height;
	ResourceDesc.Dimension = D3D12Helpers::ConvertToResourceDimension(ViewDimension);
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = Format;
	ResourceDesc.Flags = (IsDepthStencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	if (!IsDepthStencil)
	{
		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
	}
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Alignment = 0;
	ResourceDesc.DepthOrArraySize = std::max(BufferDesc.TextureDepth, 1);
	
	D3D12_RESOURCE_STATES ResourceState = IsDepthStencil ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		ResourceState,
		&ClearValue,
		IID_PPV_ARGS(&NewResource)
	));
	*Resourceptr = new GPUResource(NewResource, ResourceState);

	if (IsDepthStencil)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = Format;
		depthStencilDesc.ViewDimension = D3D12Helpers::ConvertDimensionDSV(ViewDimension);
		depthStencilDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
		depthStencilDesc.Texture2D.MipSlice = 0;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		CurrentDevice->GetDevice()->CreateDepthStencilView(NewResource, &depthStencilDesc, Heapptr->GetCPUAddress(OffsetInHeap));
		NewResource->SetName(L"FrameBuffer Stencil");
	}
	else
	{
		D3D12_RENDER_TARGET_VIEW_DESC RenderTargetDesc = {};
		RenderTargetDesc.Format = Format;
		RenderTargetDesc.ViewDimension = D3D12Helpers::ConvertDimensionRTV(ViewDimension);
		CurrentDevice->GetDevice()->CreateRenderTargetView(NewResource, &RenderTargetDesc, Heapptr->GetCPUAddress(OffsetInHeap));
		NewResource->SetName(L"FrameBuffer RT");
	}

}

void D3D12FrameBuffer::Init()
{
	if (RTVHeap == nullptr && BufferDesc.RenderTargetCount > 0)
	{
		RTVHeap = new DescriptorHeap(CurrentDevice, std::max(BufferDesc.RenderTargetCount, 1), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		RTVHeap->SetName(L"FrameBuffer RTV Heap");
	}
	if (DSVHeap == nullptr && BufferDesc.NeedsDepthStencil)
	{
		DSVHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		DSVHeap->SetName(L"FrameBuffer RTV Heap");
	}

	if (BufferDesc.NeedsDepthStencil)
	{
		CreateResource(&DepthStencil, DSVHeap, true, D3D12Helpers::ConvertFormat(BufferDesc.DepthFormat), BufferDesc.Dimension);
	}
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		CreateResource(&RenderTarget[i], RTVHeap, false, D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[i]), BufferDesc.Dimension, i);
	}
	UpdateSRV();
}

D3D12FrameBuffer::~D3D12FrameBuffer()
{
	if (BufferDesc.NeedsDepthStencil)
	{
		DepthStencil->Release();
		delete NullHeap;
		delete DSVHeap;
	}
	if (BufferDesc.RenderTargetCount > 0)
	{
		delete RTVHeap;
	}
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RenderTarget[i]->Release();
	}
	delete SrvHeap;
}

void D3D12FrameBuffer::ReadyResourcesForRead(CommandListDef * list, int Resourceindex)
{
	if (RenderTarget[Resourceindex] != nullptr)
	{
		RenderTarget[Resourceindex]->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void D3D12FrameBuffer::BindBufferToTexture(CommandListDef * list, int slot, int Resourceindex, DeviceContext* target)
{

	//ensure(Resourceindex < BufferDesc.RenderTargetCount);
	if (BufferDesc.IsShared)
	{
		MakeReadyForRead(list);
		if (target != nullptr && OtherDevice != nullptr)
		{
			if (target == OtherDevice)
			{
				SharedSRVHeap->BindHeap(list);
				SharedTarget->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				list->SetGraphicsRootDescriptorTable(slot, SharedSRVHeap->GetGpuAddress(Resourceindex));
			}
		}
		return;
	}
	lastboundslot = slot;

	SrvHeap->BindHeap(list);
	ReadyResourcesForRead(list, Resourceindex);
	list->SetGraphicsRootDescriptorTable(slot, SrvHeap->GetGpuAddress(Resourceindex));
}

void D3D12FrameBuffer::BindBufferAsRenderTarget(CommandListDef * list)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);

	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		if (RenderTarget[i] != nullptr)
		{
			RenderTarget[i]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	if (BufferDesc.NeedsDepthStencil)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE();
		if (RTVHeap)
		{
			//validate this is okay todo?
			rtvHandle = RTVHeap->GetCPUAddress(0);
		}
		list->OMSetRenderTargets(BufferDesc.RenderTargetCount, &rtvHandle, (BufferDesc.RenderTargetCount > 1), &DSVHeap->GetCPUAddress(0));
	}
	else
	{
		list->OMSetRenderTargets(BufferDesc.RenderTargetCount, &RTVHeap->GetCPUAddress(0), (BufferDesc.RenderTargetCount > 1), nullptr);
	}
}

void D3D12FrameBuffer::UnBind(CommandListDef * list)
{
	if (BufferDesc.IsShared)
	{
		TransitionTOCopy(list);
	}
	else
	{
		if (NullHeap == nullptr)
		{
			return;
		}
		NullHeap->BindHeap(list);
		NullHeap->SetName(L"null  SRV");
		//list->SetGraphicsRootDescriptorTable(lastboundslot, NullHeap->GetGpuAddress(0));
	}
}

void D3D12FrameBuffer::ClearBuffer(CommandListDef * list)
{
	if (BufferDesc.NeedsDepthStencil)
	{
		list->ClearDepthStencilView(DSVHeap->GetCPUAddress(0), D3D12_CLEAR_FLAG_DEPTH, BufferDesc.DepthClearValue, 0, 0, nullptr);
	}
	if (BufferDesc.RenderTargetCount > 0)
	{
		for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
		{
			list->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), &BufferDesc.clearcolour[0], 0, nullptr);
		}
	}
}

D3D12Shader::PipeRenderTargetDesc D3D12FrameBuffer::GetPiplineRenderDesc()
{
	D3D12Shader::PipeRenderTargetDesc output = {};
	output.NumRenderTargets = BufferDesc.RenderTargetCount;
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		output.RTVFormats[i] = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[i]);
	}
	output.DSVFormat = D3D12Helpers::ConvertFormat(BufferDesc.DepthFormat);
	return output;
}

