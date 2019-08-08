#include "ShadowAtlasStorageNode.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../../Core/ShadowAtlas.h"

ShadowAtlasStorageNode::ShadowAtlasStorageNode()
{
	StoreType = EStorageType::ShadowData;
	DataFormat = StorageFormats::ShadowData;
}


ShadowAtlasStorageNode::~ShadowAtlasStorageNode()
{}

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

ShadowAtlas * ShadowAtlasStorageNode::GetAtlas()const
{
	return Atlas;
}

void ShadowAtlasStorageNode::Create()
{
	Atlas = new ShadowAtlas(DeviceObject);
}
