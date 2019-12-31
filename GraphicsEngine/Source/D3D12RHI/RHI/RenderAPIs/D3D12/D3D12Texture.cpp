
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
#include "D3D12RHITexture.h"
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
		//TextureResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void D3D12Texture::CreateTextureFromDesc(const TextureDescription& desc)
{
	Description = desc;
	Cover = new D3D12RHITexture();
	Cover->CreateWithUpload(desc, Context);
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
	return Cover->GetDescriptor(Desc, heap);
}

DescriptorItemDesc D3D12Texture::GetItemDesc(const RHIViewDesc& Desc) const
{
	return Cover->GetItemDesc(Desc);
}

void D3D12Texture::CreateAsNull()
{

}
