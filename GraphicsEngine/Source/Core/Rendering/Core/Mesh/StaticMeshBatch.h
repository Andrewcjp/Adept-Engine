#pragma once

class LowLevelAccelerationStructure;
//this collects multiple static meshes together for faster rendering
//this is done OFFLINE.
class StaticMeshBatch
{
public:
	StaticMeshBatch();
	~StaticMeshBatch();
	void AddObject(GameObject* Mesh);
	void Update();

private:
	LowLevelAccelerationStructure* ASStruct = nullptr;
};

