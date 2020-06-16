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
#include "RHI/Streaming/TextureStreamingEngine.h"


DXGPUTextureStreamer::DXGPUTextureStreamer()
{}


DXGPUTextureStreamer::~DXGPUTextureStreamer()
{}

void DXGPUTextureStreamer::SetStreamingMode(EGPUSteamMode::Type mode)
{
	EGPUSteamMode::Type Clamp = mode;
	if (Device->GetCaps().SamplerFeedbackMode == ESamplerFeedBackSupportMode::None)
	{
		Clamp = EGPUSteamMode::None;
	}
	m_StreamingMode = Math::Min(mode, Clamp);
}

void DXGPUTextureStreamer::UpdateMappingsFromFeedback(RHICommandList* list, TextureHandle* handle)
{
	if (RHI::GetFrameCount() % 3 != 0)
	{
		return;
	}
	if (m_StreamingMode != EGPUSteamMode::SamplerFeedBack && m_StreamingMode != EGPUSteamMode::TopMipUsedBased)
	{
		return;
	}
	RHITexture* Feedback = handle->GetData(list)->Backing->PairedTexture;
	float* data = (float*)handle->m_CpuSamplerFeedBack->MapReadBack();
	const int Width = Feedback->GetDescription().Width;
	int CurrnetMip = handle->GetData(Device)->TopMipState;
	float MinMip = handle->Description.MipLevels;
	for (int i = 0; i < handle->m_CPUStreamingUpdates->GetSize(); i++)
	{
		MinMip = Math::Min(MinMip, data[i]);
		if (data[i] == 0)
		{
			float t = 0;
		}
	}
	//MinMip = data[0];
	handle->SetAllTargetMip(MinMip);
	for (int i = 0; i < handle->m_CPUStreamingUpdates->GetSize(); i++)
	{
		data[i] = CurrnetMip;
	}
	handle->m_CPUStreamingUpdates->UpdateBufferData(data, handle->m_CPUStreamingUpdates->GetSize(), EBufferResourceState::Non_PixelShader);
	handle->m_CpuSamplerFeedBack->UnMap();
}

void DXGPUTextureStreamer::Tick(RHICommandList* list)
{
	Log::LogTextToScreen("Texture Usage " + StringUtils::ByteToMB(Page->GetSizeInUse()) + " Fragmentation: " + StringUtils::ToString(Page->GetFragmentationPC()));
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
		UpdateMappingsFromFeedback(list, Handles[i]);
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
	if (m_StreamingMode == EGPUSteamMode::None)
	{
		D3D12RHITexture* Texture = new D3D12RHITexture();
		Texture->CreateWithUpload(handle->Description, Device);
		handle->GetData(Device)->Backing = Texture;
		handle->GetData(Device)->TopMipState = 0;
		handle->GetData(Device)->TargetMip = 0;
		return;
	}
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
	ImageDesc.Name = handle->Description.Name;
	Texture->Create(ImageDesc, Device);

	handle->GetData(Device)->Backing = Texture;
	handle->GetData(Device)->TopMipState = handle->Description.MipLevels;
	handle->GetData(Device)->TargetMip = 0;
	//handle->GetData(Device)->TargetMip = 4;
	if (m_StreamingMode == EGPUSteamMode::SamplerFeedBack)
	{
		handle->GetData(Device)->TargetMip = handle->Description.MipLevels;
	}

}

void DXGPUTextureStreamer::MapHandle(TextureHandle* handle, RHICommandList* list)
{
	if (m_StreamingMode == EGPUSteamMode::None)
	{
		return;
	}
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
	
	//Tex->GetResource()->SetTileMappingState(glm::ivec3(0, 0, 0), 0, true);
	//Tex->GetResource()->SetTileMappingStateUV(glm::vec3(1, 1, 0), 0, true);
	//Tex->GetResource()->SetTileMappingStateUV(glm::vec3(0.0, 0.0f, 0), 0, true);
	//Tex->GetResource()->SetTileMappingStateUV(glm::vec3(0.0, 1.0f, 0), 0, true);
	//Tex->GetResource()->SetTileMappingStateUV(glm::vec3(1.0, 0.0f, 0), 0, true);
	//Tex->GetResource()->SetTileMappingStateUV(glm::vec3(0.5, 0.5f, 0), 1, true);
	for (int i = 0; i < handle->Description.MipLevels; i++)
	{
		if (Tex->GetResource()->PackedMipsIndex <= i)
		{
			Tex->GetResource()->SetTileMappingStateForSubResource(i, true);
			break;
		}
		Tex->GetResource()->SetTileMappingStateForSubResource(i, (TargetMip <= i));
	}
	//Tex->GetResource()->SetTileMappingStateForSubResource(TargetMip, true);
	Page->MapResource2(Tex->GetResource());
	ResourceTileMapping map;
	map.FirstSubResource = TargetMip;
	map.NumSubResoruces = D.MipLevels - TargetMip;
	//Page->MapResouce(Tex->GetResource(), map);
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