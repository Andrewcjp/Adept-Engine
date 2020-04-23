#include "TextureStreamingCommon.h"
#include "../RHICommandList.h"
#include "../RHITypes.h"
#include "../Shader.h"
#include "gli/texture.hpp"
#include "gli/load.hpp"
#include "../RHITexture.h"
#include "Core/Assets/AssetManager.h"
#include "../BaseTexture.h"
#include "Rendering/Core/Defaults.h"

void TextureStreamRequest::Validate()
{
}

TextureHandle::TextureHandle()
{

}

TextureHandle::~TextureHandle()
{
}

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
		return;
	}
	Backing->SetState(List, EResourceState::PixelShader);
	List->SetTexture2(Backing, List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName(SlotName), GetCurrentView(List->GetDeviceIndex()));
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
