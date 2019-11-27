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
	if (desc.ViewType == EViewType::SRV)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d = {};
		d.Format = D3D12Helpers::ConvertFormat(Desc.Format);
		d.ViewDimension = D3D12Helpers::ConvertDimension(Desc.Dimension);
		d.Texture2D.MipLevels = Desc.MipCount;
		d.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		d.TextureCube.MostDetailedMip = desc.Mip;
		d.TextureCube.MipLevels = Math::Min(desc.MipLevels, Desc.MipCount);
		if (desc.Dimension != DIMENSION_UNKNOWN)
		{
			d.ViewDimension = D3D12Helpers::ConvertDimension(desc.Dimension);
		}
		d.Texture2DArray.ArraySize = 1;
		d.Texture2DArray.FirstArraySlice = desc.ArraySlice;
		Descriptor->CreateShaderResourceView(Resource->GetResource(), &d, 0);
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
			destTextureUAVDesc.Texture2DArray.ArraySize = 1;
			destTextureUAVDesc.Texture2DArray.FirstArraySlice = desc.ArraySlice;
		}
		Descriptor->CreateUnorderedAccessView(Resource->GetResource(), nullptr, &destTextureUAVDesc);
	}
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
	DContext->GetMemoryManager()->AllocResource(d, &Resource);
}
