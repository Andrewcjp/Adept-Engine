#pragma once

class VoxelBottomLevelAccelerationStructure;
class VoxelTopLevelAccelerationStructure;
class Scene;
class VoxelBottomLevelAccelerationStructureInstance;
class VoxelScene
{
public:
	VoxelScene();
	~VoxelScene();
	void Create(Scene* CurrnetScene);
	void SetupFromScene();
	void OnObjectAddedToScene(GameObject * object);
	void Update();

	void ProcessUpdatesToAcclerationStuctures(RHICommandList* list);
	void UpdateTopLevel(RHICommandList * List);
	VoxelTopLevelAccelerationStructure* TopLevelAcc = nullptr;
	void RenderVoxelDebug(RHICommandList* list, FrameBuffer* buffer);
	
private:
	int DebugIndex = 0;
	std::vector<VoxelBottomLevelAccelerationStructureInstance*> SceneInstances;
	std::vector<VoxelBottomLevelAccelerationStructure*> BottomStructuresToUpdate;
	Scene* pScene = nullptr;

};

