#include "D3D12RHITextureArray.h"
#include "D3D12RHI.h"
#include "RHI/RHITypes.h"
#include "D3D12CommandList.h"
#include "D3D12Framebuffer.h"
#include "DescriptorHeap.h"
#include "RHI/DeviceContext.h"
#include "DescriptorHeapManager.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Texture Array
D3D12RHITextureArray::D3D12RHITextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
{
	Device = D3D12RHI::DXConv(device);
}

D3D12RHITextureArray::~D3D12RHITextureArray()
{
}

//Add a frame buffer to this heap and ask it to create one in our heap
void D3D12RHITextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(Buffer);
	ensure(dBuffer->CheckDevice(Device->GetDeviceIndex()));
	if (slot < LinkedBuffers.size())
	{
		LinkedBuffers[slot] = dBuffer;
	}
	else
	{
		LinkedBuffers.push_back(dBuffer);
	}
}

void D3D12RHITextureArray::BindToShader(RHICommandList * list, int slot)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(list);
	ensure(DXList->GetDevice() == Device);
	DXList->SetTextureArray(this, slot, RHIViewDesc::DefaultSRV());
}

//Makes a descriptor Null Using the first frame buffers Description
void D3D12RHITextureArray::SetIndexNull(int TargetIndex, FrameBuffer* Buffer /*= nullptr*/)
{
	if (Buffer != nullptr)
	{
		D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(Buffer);
		NullHeapDesc = dBuffer->GetSrvDesc(0);
	}
	if (NullHeapDesc.Format == DXGI_FORMAT_UNKNOWN)
	{
		Log::LogMessage("Texture Array Slot Cannot be set null without format", Log::Error);
		return;
	}
	if (TargetIndex < LinkedBuffers.size())
	{
		LinkedBuffers[TargetIndex] = nullptr;
	}
}

void D3D12RHITextureArray::SetFrameBufferFormat(const RHIFrameBufferDesc & desc)
{
	NullHeapDesc = D3D12FrameBuffer::GetSrvDesc(0, desc);
}

DXDescriptor * D3D12RHITextureArray::GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap)
{
	if (heap == nullptr)
	{
		heap = Device->GetHeapManager()->GetMainHeap();
	}
	DXDescriptor* Descriptor = heap->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumEntries);
	for (int i = 0; i < NumEntries; i++)
	{
		if (i < LinkedBuffers.size() && LinkedBuffers[i] != nullptr)
		{
			D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(LinkedBuffers[i]);
			ensure(dBuffer->CheckDevice(Device->GetDeviceIndex()));
			RHIViewDesc d = RHIViewDesc::DefaultSRV();
			d.OffsetInDescriptor = i;
			dBuffer->PopulateDescriptor(Descriptor, i, d);
		}
		else if (i < LinkedTextures.size() && LinkedTextures[i] != nullptr)
		{
			RHIViewDesc d = RHIViewDesc::DefaultSRV();
			d.OffsetInDescriptor = i;
			//dBuffer->PopulateDescriptor(Descriptor, i, d);
			LinkedTextures[i]->WriteToItemDesc(Descriptor->GetItemDesc(), d);
		}
		else
		{
			Descriptor->CreateShaderResourceView(nullptr, &NullHeapDesc, i);
		}
	}
	Descriptor->Recreate();
	return Descriptor;
}

uint64 D3D12RHITextureArray::GetHash()
{
	uint64 hash = 0;
	HashUtils::hash_combine(hash, LinkedBuffers.size());
	for (int i = 0; i < LinkedBuffers.size(); i++)
	{
		if (LinkedBuffers[i] == nullptr)
		{
			continue;
		}
		RHIViewDesc d = RHIViewDesc::DefaultSRV();
		d.OffsetInDescriptor = i;
		HashUtils::hash_combine(hash, LinkedBuffers[i]->GetViewHash(d));
	}
	return hash;
}

void D3D12RHITextureArray::AddTexture(RHITexture* Tex, int Index)
{
	LinkedTextures.push_back(D3D12RHI::DXConv(Tex));
}

void D3D12RHITextureArray::SetTextureFormat(const RHITextureDesc2& Desc)
{
	NullHeapDesc.Format = D3D12Helpers::ConvertFormat(Desc.Format);
	NullHeapDesc.ViewDimension = D3D12Helpers::ConvertDimension(Desc.Dimension);
	NullHeapDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}

void D3D12RHITextureArray::Release()
{
	IRHIResourse::Release();
}

void D3D12RHITextureArray::Clear()
{
	if (LinkedBuffers.size() == 0)
	{
		return;
	}
	for (int i = 0; i < LinkedBuffers.size(); i++)
	{
		SetIndexNull(i);//clear all buffers;
	}
	LinkedBuffers.clear();
	LinkedTextures.clear();
}

