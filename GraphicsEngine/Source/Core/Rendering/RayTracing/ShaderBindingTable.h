#pragma once
#include "RHI/ShaderBase.h"
#include "RHI/RHITypes.h"
namespace GlobalRootSignatureParams
{
	enum Value
	{
		OutputViewSlot = 0,
		AccelerationStructureSlot,
		Count
	};
}
namespace DefaultLocalRootSignatureParams
{
	enum Value
	{
		IndexBuffer,
		VertexBuffer,		
		Count
	};
}
class Shader_RTBase;
struct ShaderHitGroup
{
	ShaderHitGroup(std::string name);
	std::string Name = "";
	std::wstring WName;
	Shader_RTBase* HitShader = nullptr;
	Shader_RTBase* AnyHitShader = nullptr;
	Shader_RTBase* IntersectionShader = nullptr;
};
struct GlobalRootSigniture
{
	std::vector<ShaderParameter> Params;
	std::vector<RHISamplerDesc> Samplers;
};
//this holds all shaders and their binds etc for a ray trace pass
class ShaderBindingTable
{
public:
	ShaderBindingTable();
	virtual void InitTable();
	virtual ~ShaderBindingTable();
	void RebuildHittableFromScene(Scene * Sc);
	void AddObject(GameObject * Object);
	std::vector<Shader_RTBase*> MissShaders;
	std::vector<Shader_RTBase*> RayGenShaders;
	std::vector<ShaderHitGroup*> HitGroups;
	GlobalRootSigniture GlobalRootSig;
	RHI_API void Validate();

	void ValidateShaderExports(std::map<std::string, int>& CountMap, std::vector<Shader_RTBase*>& shaders);

	void AddToMap(Shader_RTBase * shader, std::map<std::string, int>& CountMap);
	
protected:
	
	virtual void OnMeshProcessed(Mesh* Mesh, MeshEntity* E, Shader_RTBase* Shader);
};

