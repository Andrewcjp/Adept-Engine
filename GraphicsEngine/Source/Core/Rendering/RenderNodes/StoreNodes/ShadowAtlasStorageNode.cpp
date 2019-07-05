#include "ShadowAtlasStorageNode.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"

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
	Shadower->BindPointmaps(List, Slot);
}

void ShadowAtlasStorageNode::Create()
{

}
