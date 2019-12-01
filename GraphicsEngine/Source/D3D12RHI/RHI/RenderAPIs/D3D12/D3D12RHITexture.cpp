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
#include "Core\Performance\PerfManager.h"
#include "RHI\RHICommandList.h"
#include "RHI\RHIInterGPUStagingResource.h"
#include "D3D12InterGPUStagingResource.h"
#include "D3D12CommandList.h"


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


void D3D12RHITexture::CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	List->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	D3D12InterGPUStagingResource* DXres = D3D12RHI::DXConv(Res);
	D3D12CommandList* list = D3D12RHI::DXConv(List);
	D3D12DeviceContext* CurrentDevice = D3D12RHI::DXConv(Context);
	PerfManager::StartTimer("CopyToDevice");
	// Copy the intermediate render target into the shared buffer using the
	// memory layout prescribed by the render target.
	ID3D12Device* Host = CurrentDevice->GetDevice();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;
	GPUResource* TargetResource = GetResource();

	TargetResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
	DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(Desc.Format);

	CD3DX12_RESOURCE_DESC renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, Desc.Width, Desc.Height, Desc.Depth, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);

	CD3DX12_BOX box(0, 0, Desc.Width, Desc.Height);
	const int count = Desc.Depth;
	for (int i = 0; i < count; i++)
	{
		Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION dest(DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->GetResource(), renderTargetLayout);
		CD3DX12_TEXTURE_COPY_LOCATION src(TargetResource->GetResource(), i);
		list->GetCommandList()->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
	PerfManager::EndTimer("CopyToDevice");
	//DidTransferLastFrame = true;
	List->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
}

void D3D12RHITexture::CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	List->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	D3D12InterGPUStagingResource* DXres = D3D12RHI::DXConv(Res);
	D3D12CommandList* list = D3D12RHI::DXConv(List);
	PerfManager::StartTimer("MakeReadyOnTarget");
	D3D12DeviceContext* CurrentDevice = D3D12RHI::DXConv(Context);
	ID3D12Device* Host = CurrentDevice->GetDevice();
	// Copy the buffer in the shared heap into a texture that the secondary
	// adapter can sample from.	
	// Copy the shared buffer contents into the texture using the memory
	// layout prescribed by the texture.

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;
	GetResource()->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_COPY_DEST);
	DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
	GPUResource* TargetResource = GetResource();
	const int count = Desc.Depth;
	DXGI_FORMAT readFormat = D3D12Helpers::ConvertFormat(Desc.Format);

	CD3DX12_RESOURCE_DESC renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(readFormat, Desc.Width, Desc.Height, Desc.Depth, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
	for (int i = 0; i < count; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(TargetResource->GetResource(), offset);
		Host->GetCopyableFootprints(&renderTargetDesc, offset, 1, 0, &textureLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION src(DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->GetResource(), textureLayout);
		CD3DX12_BOX box(0, 0, Desc.Width, Desc.Height);
		list->GetCommandList()->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
	//int Pixelsize = (BufferDesc.SFR_FullWidth - (int)BufferDesc.ScissorRect.x)*m_height;
	//CrossGPUBytes = Pixelsize * (int)D3D12Helpers::GetBytesPerPixel(secondaryAdapterTexture.Format);

	PerfManager::EndTimer("MakeReadyOnTarget");
	//DidTransferLastFrame = true;
	List->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
}

void D3D12RHITexture::CreateWithUpload(const TextureDescription & idesc, DeviceContext * iContext)
{
	UploadDesc = idesc;
	GPUResource* textureUploadHeap;

	RHITextureDesc2 ImageDesc;
	ImageDesc.Width = UploadDesc.Width;
	ImageDesc.Height = UploadDesc.Height;
	ImageDesc.Format = UploadDesc.Format;
	ImageDesc.Dimension = DIMENSION_TEXTURE2D;
	ImageDesc.InitalState = EResourceState::CopyDst;
	ImageDesc.Depth = idesc.Faces;
	ImageDesc.MipCount = idesc.MipLevels;
	Create(ImageDesc, DContext);

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Resource->GetResource(), 0, idesc.MipLevels*idesc.Faces);

	AllocDesc D = AllocDesc();
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	D.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	DContext->GetMemoryManager()->AllocUploadTemporary(D, &textureUploadHeap);
	D3D12Helpers::NameRHIObject(textureUploadHeap, this, "(UPLOAD)");

	std::vector<D3D12_SUBRESOURCE_DATA> SubResourceDesc;
	uint64_t offset = 0;
	for (int face = 0; face < UploadDesc.Faces; face++)
	{
		for (int mip = 0; mip < UploadDesc.MipLevels; mip++)
		{
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = (void*)(offset + (UINT64)UploadDesc.PtrToData);
			textureData.RowPitch = UploadDesc.MipExtents(mip).x*UploadDesc.BitDepth;
			textureData.SlicePitch = textureData.RowPitch  * UploadDesc.MipExtents(mip).y;
			SubResourceDesc.push_back(textureData);
			offset += UploadDesc.Size(mip);
		}
	}
	GPUUploadRequest Request;
	Request.SubResourceDesc = SubResourceDesc;
	Request.UploadBuffer = textureUploadHeap;
	Request.Target = Resource;
	Request.DataPtr = UploadDesc.PtrToData;
	Request.DataPtrSize = uploadBufferSize;
	DContext->EnqueueUploadRequest(Request);

	Resource->SetName(L"Texture");
	textureUploadHeap->SetName(L"Upload");
}
