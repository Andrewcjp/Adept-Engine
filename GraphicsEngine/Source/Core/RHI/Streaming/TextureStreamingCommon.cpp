#include "TextureStreamingCommon.h"
#include "gli/load.hpp"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Core/Defaults.h"
#include "SamplerFeedbackEngine.h"
#include "Rendering/Shaders/GlobalShaderLibrary.h"

void TextureStreamRequest::Validate()
{}

TextureHandle::TextureHandle()
{

}

TextureHandle::~TextureHandle()
{}

void TextureHandle::InitFromFile(std::string file)
{
	FilePath = file;
	ValidHandle = AssetManager::Get()->ProcessTexture(this);
}

void TextureHandle::LoadToCPUMemory()
{
	AssetPathRef Fileref = AssetPathRef(FilePath);
	std::string DDCRelFilepath = "\\" + AssetManager::DDCName + "\\" + Fileref.BaseName + ".DDS";
	gli::texture tex = gli::load(AssetManager::GetRootDir() + DDCRelFilepath);
	if (tex.empty())
	{
		return;
	}
	Log::LogMessage("Loading texture " + FilePath);

	Description.Width = tex.extent().x;
	Description.Height = tex.extent().y;
	Description.MipLevels = tex.levels();
	Description.BitDepth = 4;// texChannels;
	//todo: handle this better!
	Description.PtrToData = malloc(tex.size());
	memcpy(Description.PtrToData, tex.data(), tex.size());
	Description.Faces = tex.faces();
	Description.ImageByteSize = tex.size();
	for (int i = 0; i < tex.levels(); i++)
	{
		Description.MipLevelExtents.push_back(glm::ivec2(tex.extent(i).x, tex.extent(i).y));
	}
	if (tex.target() == gli::TARGET_CUBE)
	{
		Description.TextureType = ETextureType::Type_CubeMap;
	}
	Description.Name = FilePath;
	IsCPULoaded = true;
}

void TextureHandle::LinkToMesh(MeshRendererComponent* Mesh)
{
	LinkedMesh = Mesh;
}

RHIViewDesc TextureHandle::GetCurrentView(int index)
{
	RHIViewDesc view = RHIViewDesc::DefaultSRV();
	view.MipLevels = Math::Max(Description.MipLevels - GetData(RHI::GetDeviceContext(index))->TopMipState, 1);
	view.Mip = Math::Min(GetData(RHI::GetDeviceContext(index))->TopMipState, Description.MipLevels - 1);
	return view;
}

void TextureHandle::Bind(RHICommandList* List, std::string SlotName)
{
	RHITexture* Backing = GetData(List)->Backing;
	if (Backing == nullptr || !IsValid())
	{
		List->SetTexture(Defaults::GetDefaultTexture(), SlotName);
		List->SetTexture2(Defaults::GetDefaultTexture2(), "MipClamp");
		RHIViewDesc d;
		d.DefaultUAV(DIMENSION_TEXTURE2D);
		List->SetUAV((RHITexture*)nullptr, List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("g_feedback"), d);
		return;
	}
	Backing->SetState(List, EResourceState::PixelShader);
	List->SetTexture2(Backing, List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName(SlotName), GetCurrentView(List->GetDeviceIndex()));
	if (Backing->PairedTexture != nullptr)
	{
		m_StreamedMipMap->SetState(List, EResourceState::PixelShader);
		List->SetTexture2(m_StreamedMipMap, "MipClamp");
		if (Backing->LastClearFrame != RHI::GetFrameCount())
		{
			Backing->PairedTexture->SetState(List, EResourceState::UAV);
			List->ClearUAVFloat(Backing->PairedTexture, glm::vec4(20));
			Backing->LastClearFrame = RHI::GetFrameCount();
		}
		List->SetUAV(Backing->PairedTexture, "g_feedback");
		List->UAVBarrier(Backing->PairedTexture);
	}
}

void TextureHandle::UnLoadFromCPUMemory()
{
	IsCPULoaded = false;
}

TextureHandle::PerGPUData* TextureHandle::GetData(RHICommandList* list)
{
	return &GpuData[list->GetDeviceIndex()];
}

TextureHandle::PerGPUData* TextureHandle::GetData(DeviceContext* device)
{
	return &GpuData[device->GetDeviceIndex()];
}

void TextureHandle::SetupForSFS(DeviceContext* con)
{
	m_CpuSamplerFeedBack = RHI::GetRHIClass()->CreateRHIBuffer(ERHIBufferType::ReadBack);

	SamplerFeedbackEngine::SetupPairedTexture(GetData(con)->Backing, m_CpuSamplerFeedBack);
	m_StreamedMipMap = RHI::GetRHIClass()->CreateTexture2();
	RHITextureDesc2 desc2;
	const int TileSize = RHI::GetRenderSettings()->GetSFSSettings().TileSize;
	desc2.Width = Description.Width / TileSize;
	desc2.Height = Description.Height / TileSize;
	desc2.Depth = 1;
	desc2.Format = R32_FLOAT;
	desc2.AllowUnorderedAccess = true;
	desc2.Name = "MipMaxMap for: " + StringUtils::GetFilename(FilePath.c_str());
	m_StreamedMipMap->Create(desc2);
	m_CPUStreamingUpdates = RHI::GetRHIClass()->CreateRHIBuffer(ERHIBufferType::GPU);
	RHIBufferDesc bufferdesc = m_CpuSamplerFeedBack->GetDesc();
	bufferdesc.Accesstype = EBufferAccessType::Dynamic;
	m_CPUStreamingUpdates->CreateBuffer(bufferdesc);
}

void TextureHandle::ResolveStreamingMaps(RHICommandList* list)
{
	RHITexture* Backing = GetData(list)->Backing;
	if (Backing->LastClearFrame != RHI::GetFrameCount())
	{
		Backing->PairedTexture->SetState(list, EResourceState::UAV);
		list->ClearUAVFloat(Backing->PairedTexture, glm::vec4(20));
		Backing->LastClearFrame = RHI::GetFrameCount();
	}
	list->UAVBarrier(GetData(list)->Backing->PairedTexture);
	RHITexture* Feedback = GetData(list)->Backing->PairedTexture;
	list->SetComputePipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(GlobalShaderLibrary::ResolveSamplerFeedBackShader->Get(list)));
	m_StreamedMipMap->SetState(list, EResourceState::UAV);
	list->SetUAV(m_StreamedMipMap, "MipMaxMap"); 
	list->SetUAV(Feedback, "FeedBackMap");
	
	list->SetBuffer(m_CPUStreamingUpdates, "CPU_MipData");
	list->DispatchSized(m_StreamedMipMap->GetDescription().Width, m_StreamedMipMap->GetDescription().Height, 1);
	list->UAVBarrier(m_StreamedMipMap);
	m_StreamedMipMap->SetState(list, EResourceState::PixelShader);
	ExecuteCPUReadbackAndUpdate(list);
}

void TextureHandle::ExecuteCPUReadbackAndUpdate(RHICommandList* list)
{
	//if (RHI::GetFrameCount() % 3 != 0)
	//{
	//	return;
	//}
	RHITexture* Feedback = GetData(list)->Backing->PairedTexture;
	SamplerFeedbackEngine::ResolveAndReadback(list, Feedback, m_CpuSamplerFeedBack);
}