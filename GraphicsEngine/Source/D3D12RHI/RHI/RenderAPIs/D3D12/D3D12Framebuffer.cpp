#include "D3D12Framebuffer.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "DescriptorHeap.h"
#include "GPUResource.h"
#include "DXDescriptor.h"
#include "DescriptorHeapManager.h"
#include "D3D12InterGPUStagingResource.h"
#include "DXMemoryManager.h"
#include "GPUMemoryPage.h"
#define CUBE_SIDES 6

void D3D12FrameBuffer::CreateSRVHeap(int Num)
{
	if (NullHeap == nullptr)
	{
		NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		NAME_RHI_OBJ(NullHeap);
		CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &GetSrvDesc(0), NullHeap->GetCPUAddress(0));
	}
}

void D3D12FrameBuffer::CreateSRVInHeap(int HeapOffset, DXDescriptor* desc)
{
	CreateSRVInHeap(HeapOffset, desc, CurrentDevice);
}

void D3D12FrameBuffer::CreateSRVInHeap(int HeapOffset, DXDescriptor* desc, DeviceContext* target)
{
	if (BufferDesc.RenderTargetCount > 2)
	{
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
			desc->CreateShaderResourceView(DepthStencil->GetResource(), &GetSrvDesc(0), HeapOffset);
		}
		else
		{
			shadowSrvDesc.Format = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
			desc->CreateShaderResourceView(RenderTarget[0]->GetResource(), &GetSrvDesc(0), HeapOffset);
		}
	}
}
void D3D12FrameBuffer::CreateDepthSRV(int HeapOffset, DXDescriptor* desc)
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

D3D12_SHADER_RESOURCE_VIEW_DESC D3D12FrameBuffer::GetSrvDesc(int RenderTargetIndex, const RHIFrameBufferDesc& desc)
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
		RenderTarget[i] = nullptr;
	}
	if (DepthStencil)
	{
		EnqueueSafeRHIRelease(DepthStencil);
		DepthStencil = nullptr;
	}
	Init();
}

bool D3D12FrameBuffer::IsReadyForCompute() const
{
	if (RenderTarget[0] != nullptr)
	{
		return RenderTarget[0]->GetCurrentState() & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	return false;
}

void D3D12FrameBuffer::BindDepthWithColourPassthrough(RHICommandList* List, FrameBuffer* PassThrough)
{
	D3D12FrameBuffer * DPassBuffer = D3D12RHI::DXConv(PassThrough);
	ID3D12GraphicsCommandList* list = D3D12RHI::DXConv(List)->GetCommandList();
	D3D12RHI::DXConv(List)->CurrentRenderTarget = DPassBuffer;
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

}

void D3D12FrameBuffer::CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List)
{
	ensure(OtherBuffer);
	D3D12FrameBuffer* OtherB = (D3D12FrameBuffer*)OtherBuffer;
	D3D12CommandList* CMdList = (D3D12CommandList*)List;
	D3D12_RESOURCE_DESC secondaryAdapterTexture = OtherB->RenderTarget[0]->GetResource()->GetDesc();
	OtherB->RenderTarget[0]->SetResourceState(CMdList->GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST);
	//TargetCopy->SetResourceState(CMdList->GetCommandList(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	const int count = BufferDesc.TextureDepth;
	//for (int i = 0; i < count; i++)
	//{
	//	int offset = i;
	//	CD3DX12_TEXTURE_COPY_LOCATION dest(OtherB->RenderTarget[0]->GetResource(), offset);
	//	CD3DX12_TEXTURE_COPY_LOCATION src(TargetCopy->GetResource(), offset);
	//	int PXoffset = 0;
	//	if (RHI::GetMGPUSettings()->ShowSplit)
	//	{
	//		PXoffset = 10;
	//	}
	//	CD3DX12_BOX box((LONG)BufferDesc.ScissorRect.x, 0, m_width - PXoffset, m_height);
	//	CMdList->GetCommandList()->CopyTextureRegion(&dest, (LONG)BufferDesc.ScissorRect.x + PXoffset, 0, 0, &src, &box);
	//}
}



void D3D12FrameBuffer::CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	List->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	D3D12InterGPUStagingResource* DXres = D3D12RHI::DXConv(Res);
	D3D12CommandList* list = D3D12RHI::DXConv(List);
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
	TargetResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
	DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
	if (BufferDesc.RenderTargetCount == 0)
	{
		readFormat = DXGI_FORMAT_R32_FLOAT;
	}
	renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, BufferDesc.Width, m_height, BufferDesc.TextureDepth, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);

	CD3DX12_BOX box((LONG)BufferDesc.ScissorRect.x, 0, BufferDesc.SFR_FullWidth, m_height);
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION dest(DXres->GetViewOnDevice(Device->GetDeviceIndex())->GetResource(), renderTargetLayout);
		CD3DX12_TEXTURE_COPY_LOCATION src(TargetResource->GetResource(), i);
		list->GetCommandList()->CopyTextureRegion(&dest, (LONG)BufferDesc.ScissorRect.x, 0, 0, &src, &box);
	}
	PerfManager::EndTimer("CopyToDevice");
	DidTransferLastFrame = true;
	List->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
}

void D3D12FrameBuffer::CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	List->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	D3D12InterGPUStagingResource* DXres = D3D12RHI::DXConv(Res);
	D3D12CommandList* list = D3D12RHI::DXConv(List);
	PerfManager::StartTimer("MakeReadyOnTarget");
	ID3D12Device* Host = CurrentDevice->GetDevice();
	// Copy the buffer in the shared heap into a texture that the secondary
	// adapter can sample from.	
	// Copy the shared buffer contents into the texture using the memory
	// layout prescribed by the texture.

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;
	RenderTarget[0]->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST);
	DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
	GPUResource* TargetResource = RenderTarget[0];
	const int count = BufferDesc.TextureDepth;
	DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[0]);
	if (BufferDesc.RenderTargetCount == 0)
	{
		readFormat = DXGI_FORMAT_R32_FLOAT;
	}
	renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, BufferDesc.Width, m_height, BufferDesc.TextureDepth, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
	for (int i = 0; i < count; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(TargetResource->GetResource(), offset);
		Host->GetCopyableFootprints(&renderTargetDesc, offset, 1, 0, &textureLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION src(DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->GetResource(), textureLayout);
		CD3DX12_BOX box((LONG)BufferDesc.ScissorRect.x, 0, BufferDesc.SFR_FullWidth, m_height);
		list->GetCommandList()->CopyTextureRegion(&dest, (LONG)BufferDesc.ScissorRect.x, 0, 0, &src, &box);
	}
	//int Pixelsize = (BufferDesc.SFR_FullWidth - (int)BufferDesc.ScissorRect.x)*m_height;
	//CrossGPUBytes = Pixelsize * (int)D3D12Helpers::GetBytesPerPixel(secondaryAdapterTexture.Format);

	PerfManager::EndTimer("MakeReadyOnTarget");
	DidTransferLastFrame = true;
	List->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
}

void D3D12FrameBuffer::SetState(RHICommandList* List, D3D12_RESOURCE_STATES state, bool Depth)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		GetResource(i)->SetResourceState(D3D12RHI::DXConv(List)->GetCommandList(), state);
	}
	if (Depth && DepthStencil != nullptr)
	{
		if (state == D3D12_RESOURCE_STATE_RENDER_TARGET)
		{
			state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}
		DepthStencil->SetResourceState(D3D12RHI::DXConv(List)->GetCommandList(), state);
	}
}

void D3D12FrameBuffer::SetResourceState(RHICommandList* List, EResourceState::Type State, bool ChangeDepth /*= false*/)
{
	switch (State)
	{
		case EResourceState::RenderTarget:
			SetState(List, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			break;
		case EResourceState::PixelShader:
			SetState(List, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
			break;
		case EResourceState::ComputeUse:
			SetState(List, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, ChangeDepth);
			break;
		case EResourceState::Non_PixelShader:
			SetState(List, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
			break;
		case EResourceState::UAV:
			SetState(List, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, ChangeDepth);
			break;
		case EResourceState::CopySrc:
			SetState(List, D3D12_RESOURCE_STATE_COMMON, ChangeDepth);
			break;
		case EResourceState::Limit:
			break;
	}
	CurrentState = State;
}

DXDescriptor * D3D12FrameBuffer::GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap)
{
	if (heap == nullptr)
	{
		heap = CurrentDevice->GetHeapManager()->GetMainHeap();
	}
	ensure(desc.ViewType != EViewType::Limit);
	DXDescriptor* Descriptor = heap->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	if (desc.ViewType == EViewType::SRV)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d = GetSrvDesc(desc.ResourceIndex);
		d.TextureCube.MostDetailedMip = desc.Mip;
		d.TextureCube.MipLevels = desc.MipLevels;
		if (desc.Dimension != DIMENSION_UNKNOWN)
		{
			d.ViewDimension = D3D12Helpers::ConvertDimension(desc.Dimension);
		}
		d.Texture2DArray.ArraySize = 1;
		d.Texture2DArray.FirstArraySlice = desc.ArraySlice;
		Descriptor->CreateShaderResourceView(RenderTarget[desc.ResourceIndex]->GetResource(), &d, 0);
	}
	else
	{
		ensureMsgf(BufferDesc.AllowUnorderedAccess, "Attempt to create a UAV on a framebuffer without AllowUnorderedAccess set");
		D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
		//DX12: TODO handle UAv dimentions 
		destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(GetDescription().RTFormats[0]);
		destTextureUAVDesc.Texture2D.MipSlice = desc.Mip;
		if (GetDescription().TextureDepth > 1)
		{
			destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			destTextureUAVDesc.Texture2DArray.ArraySize = 1;
			destTextureUAVDesc.Texture2DArray.FirstArraySlice = desc.ArraySlice;
		}
		Descriptor->CreateUnorderedAccessView(GetResource(0)->GetResource(), nullptr, &destTextureUAVDesc);
	}
	Descriptor->Recreate();
	return Descriptor;
}

uint64 D3D12FrameBuffer::GetInstanceHash() const
{
	uint64 hash = 0;
	HashUtils::hash_combine(hash, RenderTarget[0]);
	HashUtils::hash_combine(hash, DepthStencil);
	return hash;
}

D3D12FrameBuffer::D3D12FrameBuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{
	CurrentDevice = D3D12RHI::DXConv(device);
	Init();
	AddCheckerRef(D3D12FrameBuffer, this);
	PostInit();
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
		if (BufferDesc.AllowUnorderedAccess)
		{
			ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
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
	if (BufferDesc.AllowDynamicResize && RHI::GetRenderSettings()->GetDynamicResolutionSettings().EnableDynamicResolutionScaling)
	{
		AllocDesc Desc = AllocDesc(0, ResourceState, ResourceDesc.Flags, "FB");
		Desc.ClearValue = ClearValue;
		Desc.ResourceDesc = ResourceDesc;
		ReservedPage->Allocate(Desc, Resourceptr);
		NewResource = (*Resourceptr)->GetResource();
	}
	else
	{
		AllocDesc Desc = AllocDesc(0, ResourceState, ResourceDesc.Flags, "FB");
		Desc.ClearValue = ClearValue;
		Desc.ResourceDesc = ResourceDesc;
		CurrentDevice->GetMemoryManager()->AllocFrameBuffer(Desc, Resourceptr);
		NewResource = (*Resourceptr)->GetResource();
	}


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

		D3D12Helpers::NameRHIObject(NewResource, this, "(FB RT) fmt " + std::to_string(Format) + " MRT" + std::to_string(OffsetInHeap));
	}
#if ALLOW_RESOURCE_CAPTURE
	new D3D12ReadBackCopyHelper(CurrentDevice, *Resourceptr);
#endif
}

void D3D12FrameBuffer::Init()
{
	if (BufferDesc.AllowDynamicResize)
	{
		if (BufferDesc.MaxSize.x == 0)
		{
			BufferDesc.MaxSize.x = glm::iround(BufferDesc.Width*RHI::GetRenderSettings()->MaxRenderScale);
		}
		if (BufferDesc.MaxSize.y == 0)
		{
			BufferDesc.MaxSize.y = glm::iround(BufferDesc.Height*RHI::GetRenderSettings()->MaxRenderScale);
		}
	}
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
	if (BufferDesc.AllowDynamicResize && ReservedPage == nullptr && RHI::GetRenderSettings()->GetDynamicResolutionSettings().EnableDynamicResolutionScaling)
	{
		//#DX12: add Check for out of size resource
		// Determine how much memory is required for our resource type
		UINT64 Size = 0;
		for (int i = 0; i < MRT_MAX; i++)
		{
			D3D12_RESOURCE_ALLOCATION_INFO info = D3D12Helpers::GetResourceSizeData(BufferDesc.MaxSize.x, BufferDesc.MaxSize.y,
				D3D12Helpers::ConvertFormat(BufferDesc.RTFormats[i]), D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D /*D3D12Helpers::ConvertDimensionRTV(BufferDesc.Dimension)*/);
			Size += (info.SizeInBytes);

		}
		if (BufferDesc.NeedsDepthStencil)
		{
			D3D12_RESOURCE_ALLOCATION_INFO info = D3D12Helpers::GetResourceSizeData(BufferDesc.MaxSize.x, BufferDesc.MaxSize.y, D3D12Helpers::ConvertFormat(BufferDesc.DepthFormat),
				D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D /*D3D12Helpers::ConvertDimensionRTV(BufferDesc.Dimension)*/, true);
			Size += (info.SizeInBytes);
		}
		ReservedPage = D3D12RHI::DXConv(Device)->GetMemoryManager()->AddFrameBufferPage(Size, true);
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
	//ReadyResourcesForRead(list, Resourceindex);
	lastboundslot = slot;
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
	if (BufferDesc.AllowUnorderedAccess)
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
