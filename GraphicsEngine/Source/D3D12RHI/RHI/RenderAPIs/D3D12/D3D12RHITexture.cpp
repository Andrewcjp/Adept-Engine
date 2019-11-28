#include "D3D12RHITexture.h"
#include "RHI\RHITypes.h"
#include "D3D12Helpers.h"
#include "D3D12RHI.h"
#include "D3D12Types.h"
#include "D3D12DeviceContext.h"
#include "DXMemoryManager.h"
#include "DXDescriptor.h"
#include "DescriptorHeap.h"
#include "DescriptorHeapManager.h"
#include "GPUResource.h"
#include "Core\Maths\Math.h"
#include "D3D12Framebuffer.h"


D3D12RHITexture::D3D12RHITexture()
{}


D3D12RHITexture::~D3D12RHITexture()
{}

void D3D12RHITexture::Release()
{
	EnqueueSafeRHIRelease(Resource);
}

size_t D3D12RHITexture::GetSizeOnGPU()
{
	if (Resource != nullptr)
	{
		return  Resource->GetSizeOnGPU();
	}
	return 0;
}

DXDescriptor* D3D12RHITexture::GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap)
{
	if (heap == nullptr)
	{
		heap = DContext->GetHeapManager()->GetMainHeap();
	}
	ensure(desc.ViewType != EViewType::Limit);
	DXDescriptor* Descriptor = heap->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	WriteToDescriptor(Descriptor, desc);
	Descriptor->Recreate();
	return Descriptor;
}

void D3D12RHITexture::Create(const RHITextureDesc2& inputDesc, DeviceContext* inContext)
{
	Desc = inputDesc;
	Context = inContext;
	if (Context == nullptr)
	{
		Context = RHI::GetDefaultDevice();
	}
	DContext = D3D12RHI::DXConv(Context);

	ResourceDesc.Width = Desc.Width;
	ResourceDesc.Height = Desc.Height;
	ResourceDesc.DepthOrArraySize = Desc.Depth;
	ResourceDesc.Dimension = D3D12Helpers::ConvertToResourceDimension(Desc.Dimension);
	ResourceDesc.Format = D3D12Helpers::ConvertFormat(Desc.Format);
	ResourceDesc.MipLevels = Desc.MipCount;
	ResourceDesc.SampleDesc.Count = 1;
	if (Desc.IsRenderTarget)
	{
		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (Desc.IsDepthStencil)
	{
		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if (Desc.AllowUnorderedAccess)
	{
		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (Desc.AllowCrossGPU)
	{
		ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
	}
	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = ResourceDesc.Format;
	if (Desc.IsDepthStencil)
	{
		ClearValue.DepthStencil.Depth = Desc.DepthClearValue;
		ClearValue.DepthStencil.Stencil = 0;
	}
	else
	{
		ClearValue.Color[0] = Desc.clearcolour.r;
		ClearValue.Color[1] = Desc.clearcolour.g;
		ClearValue.Color[2] = Desc.clearcolour.b;
		ClearValue.Color[3] = Desc.clearcolour.a;
	}
	AllocDesc d;
	d.ResourceDesc = ResourceDesc;
	d.ClearValue = ClearValue;
	d.Name = Desc.Name;
	d.InitalState = D3D12FrameBuffer::ConvertState(Desc.InitalState);
	if (Desc.InitalState == EResourceState::RenderTarget && Desc.IsDepthStencil)
	{
		d.InitalState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	DContext->GetMemoryManager()->AllocResource(d, &Resource);
}

GPUResource * D3D12RHITexture::GetResource() const
{
	return Resource;
}

void D3D12RHITexture::WriteToDescriptor(DXDescriptor * Descriptor, const RHIViewDesc& desc)
{
	if (desc.ViewType == EViewType::SRV)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d = {};
		d.Format = D3D12Helpers::ConvertFormat(Desc.Format);
		if (Desc.IsDepthStencil)
		{
			d.Format = DXGI_FORMAT_R32_FLOAT;
		}
		d.ViewDimension = D3D12Helpers::ConvertDimension(Desc.Dimension);
		if (Desc.Depth > 1)
		{
			d.ViewDimension = D3D12Helpers::ConvertDimension(eTextureDimension::DIMENSION_TEXTURECUBE);
		}
		d.Texture2D.MipLevels = Desc.MipCount;
		d.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		d.TextureCube.MostDetailedMip = desc.Mip;
		d.TextureCube.MipLevels = Math::Min(desc.MipLevels, Desc.MipCount);
		if (desc.Dimension != DIMENSION_UNKNOWN)
		{
			d.ViewDimension = D3D12Helpers::ConvertDimension(desc.Dimension);
		}
		d.Texture2DArray.ArraySize = Desc.Depth;
		d.Texture2DArray.FirstArraySlice = desc.ArraySlice;
		Descriptor->CreateShaderResourceView(Resource->GetResource(), &d, desc.OffsetInDescriptor);
	}
	else
	{
		ensureMsgf(Desc.AllowUnorderedAccess, "Attempt to create a UAV on a framebuffer without AllowUnorderedAccess set");
		D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
		//DX12: TODO handle UAv dimentions 
		destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(GetDescription().Format);
		destTextureUAVDesc.Texture2D.MipSlice = desc.Mip;
		if (GetDescription().Depth > 1)
		{
			destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			destTextureUAVDesc.Texture2DArray.ArraySize = Desc.Depth;
			destTextureUAVDesc.Texture2DArray.FirstArraySlice = desc.ArraySlice;
		}
		Descriptor->CreateUnorderedAccessView(Resource->GetResource(), nullptr, &destTextureUAVDesc, desc.OffsetInDescriptor);
	}
}
