#include "DXGPUTextureStreamer.h"
#include "D3D12RHI.h"
#include "RHI/Streaming/TextureStreamingCommon.h"
#include "D3D12RHITexture.h"
#include "D3D12Types.h"
#include "DXMemoryManager.h"
#include "D3D12DeviceContext.h"
#include "GPUMemoryPage.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "D3D12CommandList.h"


DXGPUTextureStreamer::DXGPUTextureStreamer()
{
}


DXGPUTextureStreamer::~DXGPUTextureStreamer()
{
}

void DXGPUTextureStreamer::Tick(RHICommandList* list)
{
	//Log::LogTextToScreen("Texture Usage " + StringUtils::ByteToMB(Page->GetSizeInUse()) + " Fragmentation: " + StringUtils::ToString(Page->GetFragmentationPC()));
	//if (RHI::GetFrameCount() % 30 != 0)
	//{
	//	return;
	//}
	//todo!
	//update tile mappings to page
	list->ResetList();
	for (int i = 0; i < Handles.size(); i++)
	{
		MapHandle(Handles[i], list);
	}
	list->Execute();

}

void DXGPUTextureStreamer::OnInit(DeviceContext* con)
{
	Device = D3D12RHI::DXConv(con);
	//alloc GPU page(s)
	AllocDesc Desc;
	Desc.Size = Math::MBToBytes(256);
	Desc.PageAllocationType = EPageTypes::TexturesOnly;
	Desc.Name = "Texture Pool 1";
	Desc.AllowGeneralUse = false;
	Device->GetMemoryManager()->AllocPage(Desc, &Page);
}

void DXGPUTextureStreamer::OnRealiseTexture(TextureHandle* handle)
{
	//Create reserved resource 

	D3D12RHITexture* Texture = new D3D12RHITexture();
	Texture->UseReservedResouce = true;
	RHITextureDesc2 ImageDesc;
	ImageDesc.Width = handle->Description.Width;
	ImageDesc.Height = handle->Description.Height;
	ImageDesc.Format = handle->Description.Format;
	ImageDesc.Dimension = DIMENSION_TEXTURE2D;
	ImageDesc.InitalState = EResourceState::CopyDst;
	ImageDesc.Depth = handle->Description.Faces;
	ImageDesc.MipCount = handle->Description.MipLevels;
	Texture->Create(ImageDesc, Device);

	handle->GetData(Device)->Backing = Texture;
	handle->GetData(Device)->TopMipState = handle->Description.MipLevels;
	handle->GetData(Device)->TargetMip = 0;
}

void DXGPUTextureStreamer::MapHandle(TextureHandle* handle, RHICommandList* list)
{
	D3D12RHITexture* Tex = D3D12RHI::DXConv(handle->GetData(Device)->Backing);
	TextureDescription& D = handle->Description;

	if (handle->GetData(Device)->TargetMip == handle->GetData(Device)->TopMipState)
	{
		return;
	}
	const int maxMips = 1;
	int MipUpdatecount = handle->GetData(Device)->TopMipState - handle->GetData(Device)->TargetMip;
	if (MipUpdatecount > maxMips)
	{
		MipUpdatecount = maxMips;
	}
	int TargetMip = handle->GetData(Device)->TopMipState - MipUpdatecount;

	ResourceTileMapping map;
	map.FirstSubResource = TargetMip;
	map.NumSubResoruces = D.MipLevels - TargetMip;
	Page->MapResouce(Tex->GetResource(), map);
	if (MipUpdatecount < 0)
	{
		handle->GetData(Device)->TopMipState -= MipUpdatecount;
		return;
	}


	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Tex->GetResource()->GetResource(), TargetMip, MipUpdatecount);

	AllocDesc ADec = AllocDesc();
	ADec.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	ADec.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	GPUResource* textureUploadHeap;
	Device->GetMemoryManager()->AllocUploadTemporary(ADec, &textureUploadHeap);


	std::vector<D3D12_SUBRESOURCE_DATA> SubResourceDesc;
	uint64_t offset = 0;
	for (int mip = 0; mip < D.MipLevels; mip++)
	{
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = (void*)(offset + (UINT64)D.PtrToData);
		textureData.RowPitch = D.MipExtents(mip).x * D.BitDepth;
		textureData.SlicePitch = textureData.RowPitch * D.MipExtents(mip).y;
		SubResourceDesc.push_back(textureData);
		offset += D.Size(mip);
	}
	Tex->SetState(list, EResourceState::CopyDst);
	list->FlushBarriers();
	UpdateSubresources(D3D12RHI::DXConv(list)->GetCommandList(), Tex->GetResource()->GetResource(), textureUploadHeap->GetResource(), 0, TargetMip, /*SubResourceDesc.size()*/MipUpdatecount, &SubResourceDesc[TargetMip]);
	Tex->SetState(list, EResourceState::PixelShader);
	handle->GetData(Device)->TopMipState -= MipUpdatecount;
}