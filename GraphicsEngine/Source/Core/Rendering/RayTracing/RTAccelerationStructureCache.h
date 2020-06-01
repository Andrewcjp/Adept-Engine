#pragma once
class RTAccelerationStructureCache
{
public:
	LowLevelAccelerationStructure* CreateStructure(Mesh* mesh);
	LowLevelAccelerationStructure * CreateStructure(MeshEntity * mesh, Mesh * owner, int i);
private:
	std::map<uint64, LowLevelAccelerationStructure*> CacheMap;
};

