#pragma once
#include "RHI/RHI.h"
#include "ShaderBindingTable.h"

class DeviceContext;
class Shader;
class Camera;
class ShaderBindingTable;
struct ShaderHitGroupInstance;
struct ShaderHitGroup;


struct RHIStateObjectShaderRecord
{
	std::vector<ShaderHitGroup*>	HitGroups; //link any hit, closest hit and intersection
	std::vector<Shader_RTBase*>		MissShaders;
	std::vector<Shader_RTBase*>		RayGenShaders;
	GlobalRootSigniture				GlobalRS;

};

struct RHIStateObjectDesc
{
	uint AttibuteSize = 0;
	uint PayloadSize = 0;
	// PERFOMANCE TIP: Set max recursion depth as low as needed 
	// as drivers may apply optimization strategies for low recursion depths.
	uint MaxRecursionDepth = 1;
	RHIStateObjectShaderRecord ShaderRecords;
};

class RHIStateObject : public IRHIResourse
{
public:
	RHI_API RHIStateObject(DeviceContext* D, RHIStateObjectDesc Desc);
	RHI_API virtual ~RHIStateObject();
	RHI_API virtual void Build() = 0;
	ShaderBindingTable* ShaderTable = nullptr;
	RHI_API virtual void RebuildShaderTable() = 0;
	ShaderHitGroupInstance * AllocateHitGroupInstance(std::string sourceName);
protected:
	std::map<std::string, ShaderHitGroup*> ShaderHitGroups;
	DeviceContext* Device = nullptr;
	RHIStateObjectDesc Desc;
};

