#include "MeshBatch.h"
#include "MeshInstanceBuffer.h"

MeshBatch::MeshBatch()
{}


MeshBatch::~MeshBatch()
{
	MemoryUtils::DeleteVector(elements);
}

void MeshBatch::AddMeshElement(MeshBatchElement * element)
{
	elements.push_back(element);
}

void MeshBatch::Update()
{
	MainPassCulled = Owner->IsCulled(ECullingPass::MainPass);
	ShadowPassCulled = Owner->IsCulled(ECullingPass::ShadowPass);
	CastShadow = Owner->GetMesh()->GetDoesShadow();
	if (InstanceBuffer != nullptr)
	{
		InstanceBuffer->UpdateBuffer();
	}
}
