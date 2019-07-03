#include "DepthOnlyMeshProcessor.h"
#include "Core\Assets\ShaderComplier.h"
#include "RHI\RHICommandList.h"
#include "MeshDrawCommand.h"
#include "..\SceneRenderer.h"
#include "MeshBatch.h"


DepthOnlyMeshProcessor::DepthOnlyMeshProcessor()
{
	Init();
	PassType = ERenderPass::DepthOnly;
	//DisableInstancing = true;
}

DepthOnlyMeshProcessor::~DepthOnlyMeshProcessor()
{}

void DepthOnlyMeshProcessor::Init()
{

}

void DepthOnlyMeshProcessor::AddBatch(MeshBatch* Batch)
{
	Batch->Update();
	if (IsSubInstance(Batch))
	{
		return;
	}
	if (Batch->CastShadow && !Batch->ShadowPassCulled /*|| Batch->InstanceBuffer != nullptr*/)
	{
		Process(Batch);
	}
}

void DepthOnlyMeshProcessor::OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command, const MeshPassRenderArgs& args)
{

}

