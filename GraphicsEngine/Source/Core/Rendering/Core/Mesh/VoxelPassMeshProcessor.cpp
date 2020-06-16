#include "VoxelPassMeshProcessor.h"
#include "MeshDrawCommand.h"
#include "../Material.h"
#include "../Defaults.h"
#include "RHI/RHICommandList.h"
#include "RHI/Streaming/TextureStreamingCommon.h"

void VoxelPassMeshProcessor::AddBatch(MeshBatch* Batch)
{
	Process(Batch);
}

void VoxelPassMeshProcessor::OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command, const MeshPassRenderArgs& args)
{
	//if (Command->TargetMaterial != nullptr)
	//{
	//	Command->TargetMaterial->SetMaterialActive(List, args);
	//}
	//else
	//{
	//	Material::GetDefaultMaterial()->SetMaterialActive(List, args);
	//}
	//if (Command->NumInstances > 1)
	//{
	//	//todo: move this 
	//	List->SetConstantBufferView(Command->MaterialInstanceBuffer, 0, "MateralConstantBuffer");
	//}

	//List->SetTexture(Command->TargetMaterial->GetTexturebind("DiffuseMap"), "BaseTex");
	/*if (Command->TargetMaterial->TestHandle != nullptr)
	{
		Command->TargetMaterial->TestHandle->Bind(List, "BaseTex");
	}*/
	Command->TargetMaterial->ParmbindSet.BindTextures(List);
}
