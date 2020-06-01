#pragma once
#include "RHI/ShaderBase.h"
#include "RHI/RHI.h"
#include "RHI/RHITemplates.h"
#include "RHI/RHIRootSigniture.h"

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
namespace RHIHIT_GROUP_TYPE
{
	enum Type
	{
		HIT_GROUP_TYPE_TRIANGLES = 0,
		HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE = 0x1
	};
};
class Shader_RTBase;
class Shader_RTMaterialHit;
class Scene;
class GameObject;
class RHIStateObject;
struct RTSceneEntry;
struct ShaderHitGroup
{
	ShaderHitGroup(std::string name);
	std::string Name = "";
	std::wstring WName;
	Shader_RTBase* HitShader = nullptr;
	Shader_RTBase* AnyHitShader = nullptr;
	Shader_RTBase* IntersectionShader = nullptr;
	RHIHIT_GROUP_TYPE::Type			GroupType = RHIHIT_GROUP_TYPE::HIT_GROUP_TYPE_TRIANGLES;
};

struct ShaderHitGroupInstance
{
	ShaderHitGroupInstance(ShaderHitGroup* group);
	std::string Name;
	std::wstring WName;
	RHIRootSigniture mClosetHitRS;
	RHIRootSigniture mAnyHitRS;
	RHIRootSigniture mintersectionRS;
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
	void UpdateShaders(Scene * Sc);
	void AddObject(GameObject * Object);

	virtual	Shader_RTBase* GetMaterialShader();

	std::vector<Shader_RTBase*> MissShaders;
	std::vector<Shader_RTBase*> RayGenShaders;
	GlobalRootSigniture GlobalRootSig;

	RHI_API void Validate();

	void ValidateShaderExports(std::map<std::string, int>& CountMap, std::vector<Shader_RTBase*>& shaders);

	void AddToMap(Shader_RTBase * shader, std::map<std::string, int>& CountMap);

	std::vector<ShaderHitGroupInstance*> HitGroupInstances;

	RHIStateObject* LinkedStateObject = nullptr;
	void ProcessRTSceneEntity( RTSceneEntry* object);
protected:

	virtual void OnMeshProcessed(Mesh* Mesh, MeshEntity* E, ShaderHitGroupInstance* Shader);
};

