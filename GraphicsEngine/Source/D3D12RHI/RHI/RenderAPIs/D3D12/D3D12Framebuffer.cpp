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
#include "RHI/RHITexture.h"
#include "D3D12RHITexture.h"
#define CUBE_SIDES 6

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

	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		EnqueueSafeRHIRelease(RenderTarget[i]);
		BufferDesc.RenderTargets[i] = nullptr;
	}
	if (DepthStencil != nullptr)
	{
		EnqueueSafeRHIRelease(DepthStencil);
		BufferDesc.DepthStencil = nullptr;
	}
	Init();
}
void D3D12FrameBuffer::ValidateState()
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		ensure(!D3D12RHI::DXConv(BufferDesc.RenderTargets[i])->GetResource()->IsTransitioning());
	}
}
bool D3D12FrameBuffer::IsReadyForCompute() const
{
	if (GetResource(0) != nullptr)
	{
		return GetResource(0)->GetCurrentState() & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
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
	if (DPassBuffer->GetResource(0))
	{
		DPassBuffer->GetResource(0)->SetResourceState(D3D12RHI::DXConv(List), D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	if (BufferDesc.DepthStencil != nullptr)
	{
		D3D12RHI::DXConv(BufferDesc.DepthStencil)->GetResource()->SetResourceState(D3D12RHI::DXConv(List), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	list->OMSetRenderTargets(DPassBuffer->BufferDesc.RenderTargetCount, &DPassBuffer->RTVHeap->GetCPUAddress(0), true, &DSVHeap->GetCPUAddress(0));

}

DeviceContext * D3D12FrameBuffer::GetDevice()
{
	return CurrentDevice;
}

GPUResource * D3D12FrameBuffer::GetResource(int index) const
{
	return D3D12RHI::DXConv(BufferDesc.RenderTargets[index])->GetResource();
}

void D3D12FrameBuffer::Release()
{
	FrameBuffer::Release();
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12FrameBuffer, this);
	if (BufferDesc.NeedsDepthStencil)
	{
		//DepthStencil->Release();
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
	D3D12_RESOURCE_DESC secondaryAdapterTexture = OtherB->RenderTarget[0]->GetResource()->GetResource()->GetDesc();
	OtherB->RenderTarget[0]->GetResource()->SetResourceState(CMdList, D3D12_RESOURCE_STATE_COPY_DEST);
	RenderTarget[0]->GetResource()->SetResourceState(CMdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	CMdList->FlushBarriers();
	const int count = BufferDesc.TextureDepth;
	for (int i = 0; i < count; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(OtherB->RenderTarget[0]->GetResource()->GetResource(), offset);
		CD3DX12_TEXTURE_COPY_LOCATION src(RenderTarget[0]->GetResource()->GetResource(), offset);
		CD3DX12_BOX box(0, 0, BufferDesc.Width, BufferDesc.Height);
		CMdList->GetCommandList()->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
}

void D3D12FrameBuffer::SetState(RHICommandList* List, D3D12_RESOURCE_STATES state, bool Depth, EResourceTransitionMode::Type TransitionMode)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		GetResource(i)->SetResourceState(D3D12RHI::DXConv(List), state, TransitionMode);
	}
	if (Depth && BufferDesc.DepthStencil != nullptr)
	{
		if (state == D3D12_RESOURCE_STATE_RENDER_TARGET)
		{
			state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}
		if (state == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		{
			state = D3D12_RESOURCE_STATE_DEPTH_READ;
		}
		D3D12RHI::DXConv(BufferDesc.DepthStencil)->GetResource()->SetResourceState(D3D12RHI::DXConv(List), state, TransitionMode);
	}
}



void D3D12FrameBuffer::PopulateDescriptor(DXDescriptor * desc, int index, const RHIViewDesc& view)
{
	if (BufferDesc.RenderTargetCount > 0)
	{
		D3D12RHI::DXConv(BufferDesc.RenderTargets[index])->WriteToDescriptor(desc, view);
	}
	else
	{
		D3D12RHI::DXConv(BufferDesc.DepthStencil)->WriteToDescriptor(desc, view);
	}
}

uint64 D3D12FrameBuffer::GetViewHash(const RHIViewDesc & desc)
{
	uint64 Hash = 0;
	if (desc.ResourceIndex == -1 || RenderTarget[desc.ResourceIndex] == nullptr)
	{
		HashUtils::hash_combine(Hash, DepthStencil->GetItemDesc(desc).GetHash());
	}
	else
	{
		HashUtils::hash_combine(Hash, RenderTarget[desc.ResourceIndex]->GetItemDesc(desc).GetHash());
	}
	return Hash;
}

void D3D12FrameBuffer::SetResourceState(RHICommandList* List, EResourceState::Type State, bool ChangeDepth /*= false*/, EResourceTransitionMode::Type TransitionMode /*= EResourceTransitionMode::Direct*/)
{
	SetState(List, D3D12Helpers::ConvertRHIState(State), ChangeDepth, TransitionMode);
	CurrentState = State;
}

DXDescriptor * D3D12FrameBuffer::GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap)
{
	return D3D12RHI::DXConv(BufferDesc.RenderTargets[desc.ResourceIndex])->GetDescriptor(desc, heap);
}

uint64 D3D12FrameBuffer::GetInstanceHash() const
{
	uint64 hash = 0;
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		HashUtils::hash_combine(hash, RenderTarget[i]->GetResource());
	}
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

void D3D12FrameBuffer::CreateRTDescriptor(D3D12RHITexture* Texture, DescriptorHeap* Heapptr, int OffsetInHeap)
{
	const RHITextureDesc2& texDesc = Texture->GetDescription();
	if (texDesc.IsDepthStencil)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = D3D12Helpers::ConvertFormat(texDesc.Format);
		if (texDesc.DepthRenderFormat != eTEXTURE_FORMAT::FORMAT_UNKNOWN)
		{
			depthStencilDesc.Format = D3D12Helpers::ConvertFormat(texDesc.DepthRenderFormat);
		}
		depthStencilDesc.ViewDimension = D3D12Helpers::ConvertDimensionDSV(texDesc.Dimension);
		depthStencilDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
		depthStencilDesc.Texture2DArray.MipSlice = 0;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
		if (BufferDesc.CubeMapAddressAsOne)
		{
			CurrentDevice->GetDevice()->CreateDepthStencilView(Texture->GetResource()->GetResource(), &depthStencilDesc, Heapptr->GetCPUAddress(OffsetInHeap));
		}
		else
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				depthStencilDesc.Texture2DArray.ArraySize = 1;
				depthStencilDesc.Texture2DArray.FirstArraySlice = i;
				CurrentDevice->GetDevice()->CreateDepthStencilView(Texture->GetResource()->GetResource(), &depthStencilDesc, Heapptr->GetCPUAddress(OffsetInHeap + i));
			}
		}
		D3D12Helpers::NameRHIObject(Texture->GetResource(), this, "(FB Stencil)");
	}
	else
	{
		D3D12_RENDER_TARGET_VIEW_DESC RTDesc = {};
		RTDesc.Format = D3D12Helpers::ConvertFormat(texDesc.Format);
		RTDesc.ViewDimension = D3D12Helpers::ConvertDimensionRTV(texDesc.Dimension);
		if (BufferDesc.CubeMapAddressAsOne)
		{
			RTDesc.Texture2DArray.ArraySize = BufferDesc.TextureDepth;
			for (int i = 0; i < BufferDesc.MipCount; i++)
			{
				//write create rtvs for all the mips
				RTDesc.Texture2D.MipSlice = i;
				CurrentDevice->GetDevice()->CreateRenderTargetView(Texture->GetResource()->GetResource(), &RTDesc, Heapptr->GetCPUAddress(OffsetInHeap + i));
			}
		}
		else
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				RTDesc.Texture2DArray.ArraySize = 1;
				RTDesc.Texture2DArray.FirstArraySlice = i;
				CurrentDevice->GetDevice()->CreateRenderTargetView(Texture->GetResource()->GetResource(), &RTDesc, Heapptr->GetCPUAddress(OffsetInHeap + i));
			}

		}

		D3D12Helpers::NameRHIObject(Texture->GetResource(), this, "(FB RT) fmt " + std::to_string(texDesc.Format) + " MRT" + std::to_string(OffsetInHeap));
	}
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
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		BufferDesc.RenderTargets[i] = new D3D12RHITexture();
		RHITextureDesc2 Desc2;
		Desc2.Width = BufferDesc.Width;
		Desc2.Format = BufferDesc.RTFormats[i];
		Desc2.IsRenderTarget = true;
		Desc2.Height = BufferDesc.Height;
		Desc2.Depth = BufferDesc.TextureDepth;
		Desc2.MipCount = BufferDesc.MipCount;
		Desc2.InitalState = BufferDesc.SimpleStartingState;
		Desc2.AllowUnorderedAccess = BufferDesc.AllowUnorderedAccess;
		Desc2.clearcolour = BufferDesc.clearcolour;
		if (Desc2.Depth > 1)
		{
			Desc2.Dimension = DIMENSION_TEXTURECUBE;
		}
		BufferDesc.RenderTargets[i]->Create(Desc2, CurrentDevice);
	}
	if (BufferDesc.DepthFormat != eTEXTURE_FORMAT::FORMAT_UNKNOWN && BufferDesc.NeedsDepthStencil)
	{
		BufferDesc.DepthStencil = new D3D12RHITexture();
		RHITextureDesc2 Desc2;
		Desc2.Width = BufferDesc.Width;
		Desc2.Format = BufferDesc.DepthFormat;
		Desc2.IsDepthStencil = true;
		Desc2.Height = BufferDesc.Height;
		Desc2.Depth = BufferDesc.TextureDepth;
		Desc2.MipCount = BufferDesc.MipCount;
		Desc2.InitalState = EResourceState::RenderTarget;
		Desc2.DepthClearValue = BufferDesc.DepthClearValue;
		if (Desc2.Format == FORMAT_R32_TYPELESS)
		{
			Desc2.RenderFormat = eTEXTURE_FORMAT::FORMAT_R32_FLOAT;
			Desc2.DepthRenderFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
		}
		else if (Desc2.Format == FORMAT_R24_UNORM_X8_TYPELESS)
		{
			Desc2.RenderFormat = eTEXTURE_FORMAT::FORMAT_D24_UNORM_S8_UINT;
		}
		if (Desc2.Depth > 1)
		{
			Desc2.Dimension = DIMENSION_TEXTURECUBE;
		}
		BufferDesc.DepthStencil->Create(Desc2, CurrentDevice);
	}
	CreateFromTextures();
}

void D3D12FrameBuffer::CreateHeaps()
{
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
}

void D3D12FrameBuffer::CreateFromTextures()
{
	CreateHeaps();
	RenderTargetDesc.NumRenderTargets = BufferDesc.RenderTargetCount;
	for (int i = 0; i < MRT_MAX; i++)
	{
		if (BufferDesc.RenderTargets[i] == nullptr)
		{
			continue;
		}
		RenderTargetDesc.RTVFormats[i] = BufferDesc.RenderTargets[i]->GetDescription().GetRenderformat();
		RenderTarget[i] = D3D12RHI::DXConv(BufferDesc.RenderTargets[i]);
		CreateRTDescriptor(RenderTarget[i], RTVHeap, i);

	}
	if (BufferDesc.DepthStencil != nullptr)
	{
		RenderTargetDesc.DSVFormat = BufferDesc.DepthStencil->GetDescription().GetDepthRenderformat();
		DepthStencil = D3D12RHI::DXConv(BufferDesc.DepthStencil);
		CreateRTDescriptor(DepthStencil, DSVHeap, 0);
	}
}

D3D12FrameBuffer::~D3D12FrameBuffer()
{}

void D3D12FrameBuffer::BindBufferAsRenderTarget(D3D12CommandList * dxList, int SubResourceIndex)
{
	ID3D12GraphicsCommandList* list = dxList->GetCommandList();
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);

	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		if (GetResource(i) != nullptr)
		{
			GetResource(i)->SetResourceState(dxList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}
	if (BufferDesc.DepthStencil != nullptr)
	{
		D3D12RHI::DXConv(BufferDesc.DepthStencil)->GetResource()->SetResourceState(dxList, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	dxList->FlushBarriers();
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
				//		RenderTarget[i]->GetResource()->SetResourceState(list, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			}
		}
	}
}

void D3D12FrameBuffer::ClearBuffer(D3D12CommandList * list)
{
	ensure(list->GetDeviceIndex() == CurrentDevice->GetDeviceIndex());
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		if (GetResource(0) != nullptr)
		{
			GetResource(0)->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}
	if (BufferDesc.DepthStencil != nullptr)
	{
		D3D12RHI::DXConv(BufferDesc.DepthStencil)->GetResource()->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	list->FlushBarriers();
	if (BufferDesc.NeedsDepthStencil)
	{
		D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH;
		if (BufferDesc.DepthFormat == FORMAT_D24_UNORM_S8_UINT || BufferDesc.DepthFormat == FORMAT_D32_FLOAT_S8X24_UINT)
		{
			flags |= D3D12_CLEAR_FLAG_STENCIL;
		}
		if (!BufferDesc.CubeMapAddressAsOne)
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				list->GetCommandList()->ClearDepthStencilView(DSVHeap->GetCPUAddress(i), flags, BufferDesc.DepthClearValue, 0, 0, nullptr);
			}
		}
		else
		{
			list->GetCommandList()->ClearDepthStencilView(DSVHeap->GetCPUAddress(0), flags, BufferDesc.DepthClearValue, 0, 0, nullptr);
		}
	}
	if (BufferDesc.RenderTargetCount > 0)
	{
		if (!BufferDesc.CubeMapAddressAsOne)
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				list->GetCommandList()->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), &BufferDesc.clearcolour[0], 0, nullptr);
			}
		}
		else
		{
			for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
			{
				list->GetCommandList()->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), &BufferDesc.clearcolour[0], 0, nullptr);
			}
		}
	}
}
void D3D12FrameBuffer::ClearUAV(D3D12CommandList * list)
{
	if (BufferDesc.RenderTargetCount > 0)
	{
		if (!BufferDesc.CubeMapAddressAsOne)
		{
			for (int i = 0; i < BufferDesc.TextureDepth; i++)
			{
				//list->GetCommandList()->ClearUnorderedAccessViewFloat(RTVHeap->GetCPUAddress(i),);
			}
		}
		else
		{
			for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
			{
				//list->GetCommandList()->ClearUnorderedAccessViewFloat(RTVHeap->GetCPUAddress(i), &BufferDesc.clearcolour[0], 0, nullptr);
			}
		}
	}
}
const RHIPipeRenderTargetDesc& D3D12FrameBuffer::GetPiplineRenderDesc()
{
	return RenderTargetDesc;
}
