#include "ZPrePassMeshProcessor.h"
#include "Core\Assets\ShaderCompiler.h"
#include "RHI\RHICommandList.h"
#include "MeshDrawCommand.h"
#include "..\SceneRenderer.h"
#include "MeshBatch.h"


ZPrePassMeshProcessor::ZPrePassMeshProcessor()
{
	Init();
	PassType = ERenderPass::PreZ;
}

ZPrePassMeshProcessor::~ZPrePassMeshProcessor()
{}

void ZPrePassMeshProcessor::Init()
{

}
void ZPrePassMeshProcessor::AddBatch(MeshBatch* Batch)
{
	if (IsSubInstance(Batch))
	{
		return;
	}
	if (Batch->MainPassCulled && Batch->InstanceBuffer == nullptr)
	{
		return;
	}
	Process(Batch);
}

void ZPrePassMeshProcessor::OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command, const MeshPassRenderArgs& args)
{}

bool ZPrePassMeshProcessor::CheckProcess(MeshBatchElement* Element)
{
	if (Element->bTransparent)
	{
		return false;
	}
	return true;
}
