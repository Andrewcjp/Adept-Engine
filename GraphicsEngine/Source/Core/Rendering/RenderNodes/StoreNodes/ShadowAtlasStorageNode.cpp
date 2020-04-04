#include "ShadowAtlasStorageNode.h"
#include "Rendering/Core/ShadowAtlas.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHITypes.h"
#include "RHI/Shader.h"

ShadowAtlasStorageNode::ShadowAtlasStorageNode()
{
	StoreType = EStorageType::ShadowData;
	DataFormat = StorageFormats::ShadowData;
}


ShadowAtlasStorageNode::~ShadowAtlasStorageNode()
{
	SafeDelete(Atlas);
}

void ShadowAtlasStorageNode::Update()
{

}

void ShadowAtlasStorageNode::Resize()
{

}

void ShadowAtlasStorageNode::BindDirectionArray(RHICommandList * List, int Slot)
{

}

void ShadowAtlasStorageNode::BindPointArray(RHICommandList * List, int Slot)
{
	Atlas->BindPointmaps(List, Slot);
}

void ShadowAtlasStorageNode::BindPointArray(RHICommandList * List,const std::string& Slot)
{
	int slot = List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName(Slot);
	Atlas->BindPointmaps(List, slot);
}

ShadowAtlas * ShadowAtlasStorageNode::GetAtlas()const
{
	return Atlas;
}

void ShadowAtlasStorageNode::Create()
{
	Atlas = new ShadowAtlas(GetDeviceObject());
}
