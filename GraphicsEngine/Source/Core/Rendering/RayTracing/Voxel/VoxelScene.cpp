#include "VoxelScene.h"
#include "VoxelTopLevelAccelerationStructure.h"
#include "Core/Assets/Scene.h"
#include "VoxelBottomLevelAccelerationStructure.h"
#include "RHI/RHITimeManager.h"
#include "VoxelTracingEngine.h"
#include "Core/Input/Input.h"
//todo: on added to scene call backs

VoxelScene::VoxelScene()
{
}


VoxelScene::~VoxelScene()
{
}

void VoxelScene::Create(Scene * CurrnetScene)
{
	pScene = CurrnetScene;
	if (TopLevelAcc == nullptr)
	{
		TopLevelAcc = new VoxelTopLevelAccelerationStructure();
		SetupFromScene();
	}
}

void VoxelScene::SetupFromScene()
{
	for (int i = 0; i < pScene->GetMeshObjects().size(); i++)
	{
		OnObjectAddedToScene(pScene->GetMeshObjects()[i]);
	}
}

void VoxelScene::OnObjectAddedToScene(GameObject* object)
{
	if (object->GetMesh() == nullptr)
	{
		return;
	}
	//todo: cache 
	VoxelBottomLevelAccelerationStructure* Structure = new VoxelBottomLevelAccelerationStructure();
	Structure->Init(object->GetMeshRenderer());

	VoxelBottomLevelAccelerationStructureInstance* Instance = new VoxelBottomLevelAccelerationStructureInstance();
	Instance->AcclerationData = Structure;
	Instance->LinkedObject = object;
	BottomStructuresToUpdate.push_back(Structure);
	SceneInstances.push_back(Instance);
	TopLevelAcc->AddStructure(Instance);
}

void VoxelScene::Update()
{
	for (int i = 0; i < SceneInstances.size(); i++)
	{
		//todo some logic here!		
	}
	if (Input::GetKeyDown('V'))
	{
		DebugIndex++;
		DebugIndex = DebugIndex % BottomStructuresToUpdate.size();
	}
}

void VoxelScene::ProcessUpdatesToAcclerationStuctures(RHICommandList * list)
{
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "Build Voxel Structures");
		for (int i = 0; i < BottomStructuresToUpdate.size(); i++)
		{
			BottomStructuresToUpdate[i]->Build(list);
		}
		//BottomStructuresToUpdate.clear();
	}
}

void VoxelScene::UpdateTopLevel(RHICommandList* List)
{
	DECALRE_SCOPEDGPUCOUNTER(List, "Build Voxel Top Level");
	TopLevelAcc->Build(List);
}

void VoxelScene::RenderVoxelDebug(RHICommandList * list, FrameBuffer * buffer)
{
#if 0
	VoxelTracingEngine::Get()->RenderVoxelDebug(list, buffer, BottomStructuresToUpdate[DebugIndex]->GetVoxelBuffer(), BottomStructuresToUpdate[DebugIndex]->VxControlData);
#else
	VoxelTracingEngine::Get()->RenderVoxelDebug(list, buffer, TopLevelAcc->GetVoxelBuffer(), TopLevelAcc->VoxelMapControlBuffer);
	//VoxelTracingEngine::Get()->RenderVoxelDebug(list, buffer, TopLevelAcc->VoxelAlphaMap, TopLevelAcc->VoxelMapControlBuffer);
#endif
}
