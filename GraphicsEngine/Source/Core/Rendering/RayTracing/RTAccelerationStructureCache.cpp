#include "RTAccelerationStructureCache.h"
#include "Rendering/Core/Mesh.h"
#include "RayTracingEngine.h"
#include "LowLevelAccelerationStructure.h"

LowLevelAccelerationStructure * RTAccelerationStructureCache::CreateStructure(Mesh * mesh)
{
	uint64 Hash = std::hash<std::string>{}(mesh->AssetName);
	auto itor = CacheMap.find(Hash);
	if (itor != CacheMap.end())
	{
		return itor->second;
	}
	LowLevelAccelerationStructure* BLAS = RHI::GetRHIClass()->CreateLowLevelAccelerationStructure(RHI::GetDefaultDevice(), AccelerationStructureDesc());
	BLAS->CreateFromMesh(mesh);
	CacheMap.emplace(Hash, BLAS);
	//BLAS->LayerMask = GetOwner()->LayerMask.GetFlags();
	//BLAS->UpdateTransfrom(GetOwner()->GetTransform());
	//RayTracingEngine::Get()->EnqueueForBuild(BLAS);
	return BLAS;
}

LowLevelAccelerationStructure * RTAccelerationStructureCache::CreateStructure(MeshEntity * mesh, Mesh* owner, int i)
{
	uint64 Hash = std::hash<std::string>{}(owner->AssetName + std::to_string(i));
	auto itor = CacheMap.find(Hash);
	if (itor != CacheMap.end())
	{
		return itor->second;
	}

	LowLevelAccelerationStructure* BLAS = RHI::GetRHIClass()->CreateLowLevelAccelerationStructure(RHI::GetDefaultDevice(), AccelerationStructureDesc());
	BLAS->CreateFromEntity(mesh);
	CacheMap.emplace(Hash, BLAS);
	//BLAS->LayerMask = GetOwner()->LayerMask.GetFlags();
	//BLAS->UpdateTransfrom(GetOwner()->GetTransform());
	//RayTracingEngine::Get()->EnqueueForBuild(BLAS);
	return BLAS;
}
