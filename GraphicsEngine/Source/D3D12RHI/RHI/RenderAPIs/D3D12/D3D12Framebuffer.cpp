#include "D3D12Framebuffer.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "DescriptorHeap.h"
#include "GPUResource.h"
#include "Descriptor.h"
#include "DescriptorHeapManager.h"
#define CUBE_SIDES 6

void D3D12FrameBuffer::CreateSRVHeap(int Num)
{
	/*if (SrvHeap != nullptr)
	{
		SrvHeap->Release();
		delete SrvHeap;
	}*/
	//SrvHeap = new DescriptorHeap(CurrentDevice, Num, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//	NAME_RHI_OBJ(SrvHeap);
	if (NullHeap == nullptr)
	{
		NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		NAME_RHI_OBJ(NullHeap);
		CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &GetSrvDesc(0), NullHeap->GetCPUAddress(0));
	}
}

void D3D12FrameBuffer::CreateSRVInHeap(int HeapOffset, Descriptor* desc)
{
	CreateSRVInHeap(HeapOffset, desc, CurrentDevice);
}

void D3D12FrameBuffer::CreateSRVInHeap(int HeapOffset, Descriptor* desc, DeviceContext* target)
{
	if (BufferDesc.RenderTargetCount > 2)
	{
		//((D3D12DeviceContext*)target)->GetDevice()->CreateShaderResourceView(RenderTarget[HeapOffset]->GetResource(), &GetSrvDesc(HeapOffset), desc->GetCPUAddress(HeapOffset));
		desc->CreateShaderResourceView(RenderTarget[HeapOffset]->GetResource(), &GetSrvDesc(HeapOffset), HeapOffset);
	}
	else
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(BufferDesc.Dimension);
		shadowSrvDesc.Texture2D.MipLevels = 1;//BufferDesc.MipCount;
		shadowSrvDesc.Texture2DArray.MipLevels = 1;
		shadowSrvDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (BufferDesc.RenderTargetCount == 0)
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.DepthReadFormat);
			//((D3D12DeviceContext*)target)->GetDevice()->CreateShaderResourceView(DepthStencil->GetResource(), &GetSrvDesc(0), desc->GetCPUAddress(HeapOffset));
			desc->CreateShaderResourceView(DepthStencil->GetResource(), &GetSrvDesc(0), HeapOffset);
		}
		else
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
			//((D3D12DeviceContext*)target)->GetDevice()->CreateShaderResourceView(RenderTarget[0]->GetResource(), &GetSrvDesc(0), desc->GetCPUAddress(HeapOffset));
			desc->CreateShaderResourceView(RenderTarget[0]->GetResource(), &GetSrvDesc(0), HeapOffset);
		}
	}
}
void D3D12FrameBuffer::CreateDepthSRV(int HeapOffset, Descriptor* desc)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(BufferDesc.Dimension);
	shadowSrvDesc.Texture2D.MipLevels = 1;//BufferDesc.MipCount;
	shadowSrvDesc.Texture2DArray.MipLevels = 1;
	shadowSrvDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.DepthReadFormat);
	desc->CreateShaderResourceView(DepthStencil->GetResource(), &shadowSrvDesc, HeapOffset);
}

D3D12_SHADER_RESOURCE_VIEW_DESC D3D12FrameBuffer::GetSrvDesc(int RenderTargetIndex)
{
	return GetSrvDesc(RenderTargetIndex, BufferDesc);
}

D3D12_SHADER_RESOURCE_VIEW_DESC D3D12FrameBuffer::GetSrvDesc(int RenderTargetIndex, RHIFrameBufferDesc& desc)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	if (desc.RenderTargetCount > 2)
	{
		shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(desc.Dimension);
		shadowSrvDesc.Texture2D.MipLevels = desc.MipCount;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(desc.RTFormats[RenderTargetIndex]);
	}
	else
	{
		shadowSrvDesc.ViewDimension = D3D12Helpers::ConvertDimension(desc.Dimension);
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Texture2DArray.MipLevels = desc.MipCount;
		shadowSrvDesc.Texture2DArray.ArraySize = desc.TextureDepth;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (desc.RenderTargetCount == 0)
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(desc.DepthReadFormat);
		}
		else
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(desc.RTFormats[0]);
		}
	}
	return shadowSrvDesc;
}


bool D3D12FrameBuffer::CheckDevice(int index)
{
	if (Device != nullptr)
	{
		if (OtherDevice != nullptr)
		{
			return (OtherDevice->GetDeviceIndex() == index) || (Device->GetDeviceIndex() == index);
		}
		return (Device->GetDeviceIndex() == index);
	}
	return false;
}

void D3D12FrameBuffer::HandleResize()
{
	m_width = BufferDesc.Width;
	m_height = BufferDesc.Height;
	m_viewport = CD3DX12_VIEWPORT(BufferDesc.ViewPort.x, BufferDesc.ViewPort.y, BufferDesc.ViewPort.z, BufferDesc.ViewPort.w);
	m_scissorRect = CD3DX12_RECT((LONG)BufferDesc.ScissorRect.x, (LONG)BufferDesc.ScissorRect.y, (LONG)BufferDesc.ScissorRect.z, (LONG)BufferDesc.ScissorRect.w);
#if 0
	if (!RHI::GetMGPUSettings()->MainPassSFR)
	{
		CurrentDevice->CPUWaitForAll();
		if (OtherDevice != nullptr)
		{
			OtherDevice->CPUWaitForAll();
		}
	}
#endif
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		EnqueueSafeRHIRelease(RenderTarget[i]);
	}
	if (DepthStencil)
	{
		EnqueueSafeRHIRelease(DepthStencil);
	}
	if (OtherDevice != nullptr)
	{
		EnqueueSafeRHIRelease(TargetCopy);
		D3D12RHI::Get()->AddObjectToDeferredDeleteQueue(TWO_CrossHeap);
		D3D12RHI::Get()->AddObjectToDeferredDeleteQueue(FinalOut);
#if 0
		//#DX12 Fix this resource allocation issue.
		//SafeRelease(TWO_CrossHeap);
		SafeRelease(CrossHeap);
		SafeRelease(PrimaryRes);
		SafeRelease(FinalOut);
		SafeRelease(Stagedres);
		SafeRelease(SharedSRVHeap);
#endif
	}
	RHI::WaitForGPU();
	Init();
	if (OtherDevice != nullptr)
	{
		SetupCopyToDevice(OtherDevice);
	}
	RHI::WaitForGPU();
}

bool D3D12FrameBuffer::IsReadyForCompute() const
{
	if (RenderTarget[0] != nullptr)
	{
		return RenderTarget[0]->GetCurrentState() == D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	return false;
}

void D3D12FrameBuffer::SetupCopyToDevice(DeviceContext * device)
{
	ensure(device != CurrentDevice);
	OtherDevice = (D3D12DeviceContext*)device;
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();

	DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
	if (BufferDesc.RenderTargetCount == 0)
	{
		readFormat = DXGI_FORMAT_R32_FLOAT;
	}
	renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, BufferDesc.Width, m_height, BufferDesc.TextureDepth, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
	UINT64 pTotalBytes = 0;
	Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &layout, nullptr, nullptr, &pTotalBytes);
	UINT64 textureSize = D3D12Helpers::Align(layout.Footprint.RowPitch * layout.Footprint.Height);
	CrossGPUBytes = (int)textureSize;
	// Create a buffer with the same layout as the render target texture.
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
	// We can close the handle after opening the cross-adapter shared resource.
	CloseHandle(heapHandle);
	if (PrimaryRes != nullptr)
	{
		D3D12RHI::Get()->AddObjectToDeferredDeleteQueue(PrimaryRes);
	}
	//target
	ThrowIfFailed(Host->CreatePlacedResource(
		CrossHeap,
		0,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&PrimaryRes)));
	NAME_RHI_OBJ(PrimaryRes);
	//source
	if (Stagedres != nullptr)
	{
		D3D12RHI::Get()->AddObjectToDeferredDeleteQueue(Stagedres);
	}
	ThrowIfFailed(Target->CreatePlacedResource(
		TWO_CrossHeap,
		0,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		nullptr,
		IID_PPV_ARGS(&Stagedres)));
	NAME_RHI_OBJ(Stagedres);
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
	if (SharedSRVHeap == nullptr)
	{
		//SharedSRVHeap = new DescriptorHeap(OtherDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		SharedSRVHeap = OtherDevice->GetHeapManager()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	}
	TargetCopy = new GPUResource(FinalOut, D3D12_RESOURCE_STATE_COPY_DEST, OtherDevice);
	NAME_RHI_OBJ(TargetCopy);
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

void D3D12FrameBuffer::CopyToHostMemory(ID3D12GraphicsCommandList* list)
{
	PerfManager::StartTimer("CopyToDevice");
	// Copy the intermediate render target into the shared buffer using the
	// memory layout prescribed by the render target.
	ID3D12Device* Host = CurrentDevice->GetDevice();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;
	GPUResource* TargetResource = RenderTarget[0];
	if (BufferDesc.RenderTargetCount == 0)
	{
		TargetResource = DepthStencil;
	}
	TargetResource->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_SOURCE);

	CD3DX12_BOX box((LONG)BufferDesc.ScissorRect.x, 0, BufferDesc.SFR_FullWidth, m_height);
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION dest(PrimaryRes, renderTargetLayout);
		CD3DX12_TEXTURE_COPY_LOCATION src(TargetResource->GetResource(), i);
		list->CopyTextureRegion(&dest, (LONG)BufferDesc.ScissorRect.x, 0, 0, &src, &box);
	}
	PerfManager::EndTimer("CopyToDevice");
	DidTransferLastFrame = true;
}

void D3D12FrameBuffer::CopyFromHostMemory(ID3D12GraphicsCommandList* list)
{

	PerfManager::StartTimer("MakeReadyOnTarget");
	ID3D12Device* Host = CurrentDevice->GetDevice();
	// Copy the buffer in the shared heap into a texture that the secondary
	// adapter can sample from.	
	// Copy the shared buffer contents into the texture using the memory
	// layout prescribed by the texture.
	D3D12_RESOURCE_DESC secondaryAdapterTexture = FinalOut->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;
	TargetCopy->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(FinalOut, offset);
		Host->GetCopyableFootprints(&secondaryAdapterTexture, offset, 1, 0, &textureLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION src(Stagedres, textureLayout);
		CD3DX12_BOX box((LONG)BufferDesc.ScissorRect.x, 0, BufferDesc.SFR_FullWidth, m_height);
		list->CopyTextureRegion(&dest, (LONG)BufferDesc.ScissorRect.x, 0, 0, &src, &box);
	}
	int Pixelsize = (BufferDesc.SFR_FullWidth - (int)BufferDesc.ScissorRect.x)*m_height;
	CrossGPUBytes = Pixelsize * (int)D3D12Helpers::GetBytesPerPixel(secondaryAdapterTexture.Format);

	PerfManager::EndTimer("MakeReadyOnTarget");
	DidTransferLastFrame = true;
}

void D3D12FrameBuffer::MakeReadyForRead(ID3D12GraphicsCommandList * list)
{
	TargetCopy->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void D3D12FrameBuffer::MakeReadyForCopy(RHICommandList * list)
{
	if (BufferDesc.IsShared)
	{
		ensure(Device != list->GetDevice());//Make ready for copy only applies to the target GPU
	}
	MakeReadyForCopy_In(((D3D12CommandList*)list)->GetCommandList());
}

void D3D12FrameBuffer::MakeReadyForCopy_In(ID3D12GraphicsCommandList * list)
{
	if (BufferDesc.IsShared)
	{
		TargetCopy->SetResourceState(list, D3D12_RESOURCE_STATE_COMMON);//D3D12_RESOURCE_STATE_COPY_DEST
	}
	else
	{
		RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_COMMON);
	}
}

void D3D12FrameBuffer::MakeReadyForComputeUse(RHICommandList * List, bool Depth)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		GetResource(i)->SetResourceState(((D3D12CommandList*)List)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	if (Depth)
	{
		DepthStencil->SetResourceState(((D3D12CommandList*)List)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
}

void D3D12FrameBuffer::BindDepthWithColourPassthrough(RHICommandList* List, FrameBuffer* PassThrough)
{
	D3D12FrameBuffer * DPassBuffer = (D3D12FrameBuffer*)PassThrough;
	ID3D12GraphicsCommandList* list = ((D3D12CommandList*)List)->GetCommandList();
	((D3D12CommandList*)List)->CurrentRenderTarget = DPassBuffer;
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
	if (DPassBuffer->RenderTarget[0])
	{
		DPassBuffer->RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	list->OMSetRenderTargets(DPassBuffer->BufferDesc.RenderTargetCount, &DPassBuffer->RTVHeap->GetCPUAddress(0), true, &DSVHeap->GetCPUAddress(0));

}

DeviceContext * D3D12FrameBuffer::GetTargetDevice()
{
	return OtherDevice;
}

DeviceContext * D3D12FrameBuffer::GetDevice()
{
	return CurrentDevice;
}

GPUResource * D3D12FrameBuffer::GetResource(int index)
{
	return RenderTarget[index];
}

void D3D12FrameBuffer::Release()
{
	FrameBuffer::Release();
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12FrameBuffer, this);
	if (BufferDesc.NeedsDepthStencil)
	{
		DepthStencil->Release();
		SafeRelease(DSVHeap);
	}
	SafeRelease(NullHeap);
	if (BufferDesc.RenderTargetCount > 0)
	{
		SafeRelease(RTVHeap);
	}
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RenderTarget[i]->Release();
	}
	//	SafeRelease(SrvHeap);
	SafeRelease(PrimaryRes);
	SafeRelease(Stagedres);
	SafeRelease(FinalOut);
	SafeRelease(SharedSRVHeap);
	SafeRelease(TargetCopy);
	SafeRelease(TWO_CrossHeap);
	SafeRelease(CrossHeap);
}

void D3D12FrameBuffer::CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List)
{
	ensure(OtherBuffer);
	D3D12FrameBuffer* OtherB = (D3D12FrameBuffer*)OtherBuffer;
	D3D12CommandList* CMdList = (D3D12CommandList*)List;
	D3D12_RESOURCE_DESC secondaryAdapterTexture = OtherB->RenderTarget[0]->GetResource()->GetDesc();
	OtherB->RenderTarget[0]->SetResourceState(CMdList->GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST);
	TargetCopy->SetResourceState(CMdList->GetCommandList(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(OtherB->RenderTarget[0]->GetResource(), offset);
		CD3DX12_TEXTURE_COPY_LOCATION src(TargetCopy->GetResource(), offset);
		int PXoffset = 0;
		if (RHI::GetMGPUSettings()->ShowSplit)
		{
			PXoffset = 10;
		}
		CD3DX12_BOX box((LONG)BufferDesc.ScissorRect.x, 0, m_width - PXoffset, m_height);
		CMdList->GetCommandList()->CopyTextureRegion(&dest, (LONG)BufferDesc.ScissorRect.x + PXoffset, 0, 0, &src, &box);
	}
}



D3D12FrameBuffer::D3D12FrameBuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{
	CurrentDevice = (D3D12DeviceContext*)device;
	Init();
	if (BufferDesc.IsShared)
	{
		SetupCopyToDevice(BufferDesc.DeviceToCopyTo);
	}
	AddCheckerRef(D3D12FrameBuffer, this);
	PostInit();
}

void D3D12FrameBuffer::UpdateSRV()
{
	if (SRVDesc == nullptr)
	{
		SRVDesc = CurrentDevice->GetHeapManager()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, std::max(BufferDesc.RenderTargetCount + 1, 2));
	}

	if (NullHeap == nullptr)
	{
		NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		NAME_RHI_OBJ(NullHeap);
	}
	if (BufferDesc.RenderTargetCount == 0)
	{
		CreateSRVInHeap(0, SRVDesc);
	}
	else
	{
		for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
		{
			CreateSRVInHeap(i, SRVDesc);
		}
	}
	if (BufferDesc.NeedsDepthStencil)
	{
		CreateDepthSRV(BufferDesc.RenderTargetCount, SRVDesc);
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
	ResourceDesc.Format = Format;
	ResourceDesc.Flags = (IsDepthStencil ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	if (!IsDepthStencil)
	{
		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		ResourceDesc.MipLevels = BufferDesc.MipCount;
	}
	else
	{
		ResourceDesc.MipLevels = BufferDesc.DepthMipCount;
	}
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Alignment = 0;
	ResourceDesc.DepthOrArraySize = std::max(BufferDesc.TextureDepth, 1);

	D3D12_RESOURCE_STATES ResourceState = IsDepthStencil ? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_RENDER_TARGET;
	if (BufferDesc.StartingState != D3D12_RESOURCE_STATE_COMMON)
	{
		ResourceState = (D3D12_RESOURCE_STATES)BufferDesc.StartingState;
	}
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		ResourceState,
		&ClearValue,
		IID_PPV_ARGS(&NewResource)
	));
	*Resourceptr = new GPUResource(NewResource, ResourceState, Device);

	if (IsDepthStencil)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = Format;
		depthStencilDesc.ViewDimension = D3D12Helpers::ConvertDimensionDSV(ViewDimension);
		depthStencilDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
		depthStencilDesc.Texture2DArray.MipSlice = 0;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		if (BufferDesc.CubeMapAddressAsOne)
		{
			CurrentDevice->GetDevice()->CreateDepthStencilView(NewResource, &depthStencilDesc, Heapptr->GetCPUAddress(OffsetInHeap));
		}
		else
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				depthStencilDesc.Texture2DArray.ArraySize = 1;
				depthStencilDesc.Texture2DArray.FirstArraySlice = i;
				CurrentDevice->GetDevice()->CreateDepthStencilView(NewResource, &depthStencilDesc, Heapptr->GetCPUAddress(OffsetInHeap + i));
			}
		}
		D3D12Helpers::NameRHIObject(NewResource, this, "(FB Stencil)");
	}
	else
	{
		D3D12_RENDER_TARGET_VIEW_DESC RTDesc = {};
		RTDesc.Format = Format;
		RTDesc.ViewDimension = D3D12Helpers::ConvertDimensionRTV(ViewDimension);
		if (BufferDesc.CubeMapAddressAsOne)
		{
			RTDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
			for (int i = 0; i < BufferDesc.MipCount; i++)
			{
				//write create rtvs for all the mips
				RTDesc.Texture2D.MipSlice = i;
				CurrentDevice->GetDevice()->CreateRenderTargetView(NewResource, &RTDesc, Heapptr->GetCPUAddress(OffsetInHeap + i));
			}
		}
		else
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				RTDesc.Texture2DArray.ArraySize = 1;
				RTDesc.Texture2DArray.FirstArraySlice = i;
				CurrentDevice->GetDevice()->CreateRenderTargetView(NewResource, &RTDesc, Heapptr->GetCPUAddress(OffsetInHeap + i));
			}

		}

		D3D12Helpers::NameRHIObject(NewResource, this, "(FB RT)");
	}
#if ALLOW_RESOURCE_CAPTURE
	new D3D12ReadBackCopyHelper(CurrentDevice, *Resourceptr);
#endif
}

void D3D12FrameBuffer::Init()
{
	m_viewport = CD3DX12_VIEWPORT(BufferDesc.ViewPort.x, BufferDesc.ViewPort.y, BufferDesc.ViewPort.z, BufferDesc.ViewPort.w);
	m_scissorRect = CD3DX12_RECT((LONG)BufferDesc.ScissorRect.x, (LONG)BufferDesc.ScissorRect.y, (LONG)BufferDesc.ScissorRect.z, (LONG)BufferDesc.ScissorRect.w);
	//update RenderTargetDesc
	RenderTargetDesc.NumRenderTargets = BufferDesc.RenderTargetCount;
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RenderTargetDesc.RTVFormats[i] = BufferDesc.RTFormats[i];
	}
	RenderTargetDesc.DSVFormat = BufferDesc.DepthFormat;

	int Descriptorcount = std::max(BufferDesc.RenderTargetCount + BufferDesc.MipCount, 1);
	if (!BufferDesc.CubeMapAddressAsOne)
	{
		Descriptorcount = std::max(BufferDesc.RenderTargetCount*BufferDesc.TextureDepth, 1);
	}
	if (RTVHeap == nullptr && BufferDesc.RenderTargetCount > 0)
	{
		RTVHeap = new DescriptorHeap(CurrentDevice, Descriptorcount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		NAME_RHI_OBJ(RTVHeap);
	}
	if (DSVHeap == nullptr && BufferDesc.NeedsDepthStencil)
	{
		DSVHeap = new DescriptorHeap(CurrentDevice, Descriptorcount, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		NAME_RHI_OBJ(DSVHeap);
	}

	if (BufferDesc.NeedsDepthStencil)
	{
		CreateResource(&DepthStencil, DSVHeap, true, D3D12Helpers::ConvertFormat(BufferDesc.DepthFormat), BufferDesc.Dimension);
	}
	if (BufferDesc.RenderTargetCount > 0)
	{
		const int MipStride = BufferDesc.MipCount;
		for (int i = 0; i < BufferDesc.RenderTargetCount; i += MipStride)
		{
			CreateResource(&RenderTarget[i], RTVHeap, false, D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[i / BufferDesc.MipCount]), BufferDesc.Dimension, i);
		}
	}
	UpdateSRV();
}

D3D12FrameBuffer::~D3D12FrameBuffer()
{}

void D3D12FrameBuffer::ReadyResourcesForRead(ID3D12GraphicsCommandList * list, int Resourceindex)
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

void D3D12FrameBuffer::BindBufferToTexture(ID3D12GraphicsCommandList * list, int slot, int Resourceindex, DeviceContext* target, bool isCompute)
{

	if (BufferDesc.IsShared)
	{
		MakeReadyForRead(list);
		if (target != nullptr && OtherDevice != nullptr)
		{
			if (target == OtherDevice)
			{
				TargetCopy->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				list->SetGraphicsRootDescriptorTable(slot, SharedSRVHeap->GetGPUAddress(Resourceindex));
			}
		}
		return;
	}
	//UpdateSRV();
	//	ensure(Resourceindex < BufferDesc.RenderTargetCount);
	lastboundslot = slot;
	//SrvHeap->BindHeap_Old(list);
	if (isCompute)
	{
		if (Resourceindex == -1)
		{
			list->SetComputeRootDescriptorTable(slot, SRVDesc->GetGPUAddress(BufferDesc.RenderTargetCount));
		}
		else
		{
			list->SetComputeRootDescriptorTable(slot, SRVDesc->GetGPUAddress(Resourceindex));
		}
	}
	else
	{
		ReadyResourcesForRead(list, Resourceindex);
		list->SetGraphicsRootDescriptorTable(slot, SRVDesc->GetGPUAddress(Resourceindex));
	}
}

void D3D12FrameBuffer::BindBufferAsRenderTarget(ID3D12GraphicsCommandList * list, int SubResourceIndex)
{
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
		if (BufferDesc.CubeMapAddressAsOne)
		{
			list->OMSetRenderTargets(BufferDesc.RenderTargetCount, &rtvHandle, (BufferDesc.RenderTargetCount > 1), &DSVHeap->GetCPUAddress(0));
		}
		else
		{
			list->OMSetRenderTargets(BufferDesc.RenderTargetCount, &RTVHeap->GetCPUAddress(SubResourceIndex), false, &DSVHeap->GetCPUAddress(SubResourceIndex));
		}
	}
	else
	{
		list->OMSetRenderTargets(BufferDesc.RenderTargetCount, &RTVHeap->GetCPUAddress(0), (BufferDesc.RenderTargetCount > 1), nullptr);
	}
}

void D3D12FrameBuffer::UnBind(ID3D12GraphicsCommandList * list)
{
	if (BufferDesc.IsShared)
	{
		TransitionTOCopy(list);
	}
	else
	{
		if (BufferDesc.AllowUnordedAccess)
		{
			for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
			{
				if (RenderTarget[i] != nullptr)
				{
					RenderTarget[i]->SetResourceState(list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				}
			}
		}
	}
}

void D3D12FrameBuffer::ClearBuffer(ID3D12GraphicsCommandList * list)
{
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
		if (!BufferDesc.CubeMapAddressAsOne)
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				list->ClearDepthStencilView(DSVHeap->GetCPUAddress(i), D3D12_CLEAR_FLAG_DEPTH, BufferDesc.DepthClearValue, 0, 0, nullptr);
			}
		}
		else
		{
			list->ClearDepthStencilView(DSVHeap->GetCPUAddress(0), D3D12_CLEAR_FLAG_DEPTH, BufferDesc.DepthClearValue, 0, 0, nullptr);
		}
	}
	if (BufferDesc.RenderTargetCount > 0)
	{
		if (!BufferDesc.CubeMapAddressAsOne)
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				list->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), &BufferDesc.clearcolour[0], 0, nullptr);
			}
		}
		else
		{
			for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
			{
				list->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), &BufferDesc.clearcolour[0], 0, nullptr);
			}
		}
	}
}

const RHIPipeRenderTargetDesc& D3D12FrameBuffer::GetPiplineRenderDesc()
{
	return RenderTargetDesc;
}