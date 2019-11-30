
#include <iostream>
#include "Core/Assets/ImageIO.h"
#include "Core/Engine.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/FileUtils.h"
#include "GPUResource.h"
#include "ThirdParty/NVDDS/DDSTextureLoader12.h"
#include "DescriptorHeap.h"
#include "D3D12Texture.h"
#include "D3D12DeviceContext.h"
#include "D3D12CommandList.h"
#include "DescriptorHeapManager.h"
#include "DXMemoryManager.h"
#include "DXDescriptor.h"
#include "Core/Maths/Math.h"
CreateChecker(D3D12Texture);
D3D12Texture::D3D12Texture(DeviceContext* inDevice)
{
	AddCheckerRef(D3D12Texture, this);
	Context = inDevice;
	if (inDevice == nullptr)
	{
		Device = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	}
	else
	{
		Device = D3D12RHI::DXConv(inDevice);
	}
	FrameCreated = RHI::GetFrameCount();
	if (FrameCreated == 0)
	{
		FrameCreated = -10;
	}
}

void D3D12Texture::Release()
{
	SafeRelease(m_texture);
	SafeRelease(TextureResource);
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12Texture, this);
}

D3D12Texture::~D3D12Texture()
{}

void D3D12Texture::BindToSlot(D3D12CommandList* list, int slot)
{
	if (RHI::GetFrameCount() > FrameCreated + 1)
	{
		TextureResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void D3D12Texture::CreateTextureFromDesc(const TextureDescription& desc)
{
	Description = desc;
	GPUResource* textureUploadHeap;
	// Describe and create a Texture2D.
	D3D12_RESOURCE_DESC textureDesc = {};

	textureDesc.MipLevels = desc.MipLevels;
	textureDesc.Width = desc.Width;
	textureDesc.Height = desc.Height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	textureDesc.DepthOrArraySize = desc.Faces;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	if (UsingDDSLoad)
	{
		textureDesc.Format = format;
	}
	else
	{
		textureDesc.Format = D3D12Helpers::ConvertFormat(desc.Format);
	}
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	AllocDesc D;
	D.ResourceDesc = textureDesc;
	D.InitalState = D3D12_RESOURCE_STATE_COPY_DEST;
	Device->GetMemoryManager()->AllocTexture(D, &TextureResource);
	m_texture = TextureResource->GetResource();
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, desc.MipLevels*desc.Faces);

	D = AllocDesc();
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	D.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	Device->GetMemoryManager()->AllocUploadTemporary(D, &textureUploadHeap);
	D3D12Helpers::NameRHIObject(textureUploadHeap, this, "(UPLOAD)");
	std::vector<D3D12_SUBRESOURCE_DATA> SubResourceDesc;
	uint64_t offset = 0;
	for (int face = 0; face < desc.Faces; face++)
	{
		for (int mip = 0; mip < desc.MipLevels; mip++)
		{
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = (void*)(offset + (UINT64)desc.PtrToData);
			textureData.RowPitch = Description.MipExtents(mip).x*desc.BitDepth;
			textureData.SlicePitch = textureData.RowPitch  * Description.MipExtents(mip).y;
			SubResourceDesc.push_back(textureData);
			offset += Description.Size(mip);
		}
	}

	UpdateSubresources(Device->GetCopyList(), m_texture, textureUploadHeap->GetResource(), 0, 0, SubResourceDesc.size(), &SubResourceDesc[0]);

	RHI::AddToDeferredDeleteQueue(textureUploadHeap);
	Device->NotifyWorkForCopyEngine();
	m_texture->SetName(L"Texture");
	textureUploadHeap->SetName(L"Upload");
	MipLevelsReadyNow = desc.MipLevels;
}

GPUResource * D3D12Texture::GetResource()
{
	return TextureResource;
}

bool D3D12Texture::CheckDevice(int index)
{
	if (Device != nullptr)
	{
		return (Device->GetDeviceIndex() == index);
	}
	return false;
}

DXDescriptor * D3D12Texture::GetDescriptor(RHIViewDesc Desc, DescriptorHeap* heap)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (UsingDDSLoad)
	{
		srvDesc.Format = format;
	}
	else
	{
		srvDesc.Format = D3D12Helpers::ConvertFormat(Description.Format);
	}
	if (MaxMip != -1)
	{
		MipLevelsReadyNow = 1;
	}
	if (CurrentTextureType == ETextureType::Type_CubeMap)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels =Math::Min(Desc.MipLevels, Description.MipLevels - 1);
		srvDesc.TextureCube.MostDetailedMip = Math::Min(Desc.Mip, Description.MipLevels - 1);
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = Math::Min(Desc.MipLevels, Description.MipLevels-1);
		srvDesc.Texture2D.MostDetailedMip = Math::Min(Desc.Mip, Description.MipLevels - 1);
	}
	DXDescriptor* output = new DXDescriptor();
	if (heap == nullptr)
	{
		heap = Device->GetHeapManager()->GetMainHeap();
	}
	output = heap->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	output->CreateShaderResourceView(m_texture, &srvDesc);
	output->Recreate();
	return output;
}

void D3D12Texture::CreateAsNull()
{

}
