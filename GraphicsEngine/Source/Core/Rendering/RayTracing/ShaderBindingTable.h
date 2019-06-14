#pragma once
#include "RHI/ShaderBase.h"
#include "RHI/RHITypes.h"

class Shader_RTBase;
struct ShaderHitGroup
{
	ShaderHitGroup(std::string name)
	{
		Name = name;
	}
	std::string Name = "";
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

	void InitDefault();

	void InitReflections();

	~ShaderBindingTable();
	void RebuildHittableFromScene(Scene * Sc);
	void AddObject(GameObject * Object);
	std::vector<Shader_RTBase*> MissShaders;
	std::vector<Shader_RTBase*> RayGenShaders;
	std::vector<ShaderHitGroup*> HitGroups;
	GlobalRootSigniture GlobalRootSig;

};

