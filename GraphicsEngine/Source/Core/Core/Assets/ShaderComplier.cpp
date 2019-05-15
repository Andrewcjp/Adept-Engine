#include "ShaderComplier.h"
#include "Asset_Shader.h"
#include "Core/Engine.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/Shader.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "Rendering/ShaderGraph/ShaderGraphComplier.h"
ShaderComplier * ShaderComplier::Instance = nullptr;
static ConsoleVariable GenDebugShaders("DebugShaders", 0, ECVarType::LaunchOnly);
ShaderComplier::ShaderComplier()
{
	MaterialCompiler = new ShaderGraphComplier();
}

ShaderComplier::~ShaderComplier()
{}

ShaderComplier * ShaderComplier::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ShaderComplier();
	}
	return Instance;
}

void ShaderComplier::ComplieAllGlobalShaders()
{
	SCOPE_STARTUP_COUNTER("ComplieAllGlobalShaders");
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderMap.begin(); it != GlobalShaderMap.end(); ++it)
	{
#if !BASIC_RENDER_ONLY
		ComplieShader(it->second, RHI::GetDefaultDevice());
#endif
	}
}

void ShaderComplier::FreeAllGlobalShaders()
{
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderMap.begin(); it != GlobalShaderMap.end(); ++it)
	{
		SafeDelete(it->second.CompliedShader);
	}
}

bool ShaderComplier::ShouldBuildDebugShaders()
{
#if _DEBUG && WITH_EDITOR
	//return !Engine::GetIsCooking();
	return false;
#else
	return GenDebugShaders.GetBoolValue();
#endif	
}

void ShaderComplier::ComplieShader(ShaderType & type, DeviceContext* Context)
{
	if (type.CompliedShader == nullptr)
	{
		type.ShaderInitalizer.Context = Context;
		type.CompliedShader = type.Constructor(type.ShaderInitalizer);
	}
}
//#Shader_Complier compile materials too
//#Shader_Complier device index
ShaderType* ShaderComplier::GetShaderFromGlobalMap(std::string name)
{
	StringUtils::RemoveChar(name, "class ");
	if (GlobalShaderMap.find(name) != GlobalShaderMap.end())
	{
		return &GlobalShaderMap.at(name);
	}
	DebugEnsure(false);
	return nullptr;
}

void ShaderComplier::AddShaderType(std::string Name, ShaderType type)
{
	GlobalShaderMap.emplace(Name, type);
}

void ShaderComplier::TickMaterialComplie()
{
	if (MaterialShaderComplieQueue.empty())
	{
		return;
	}
	for (int i = 0; i < MaterialShaderComplieQueue.size(); i++)
	{
		ComplieMateral(MaterialShaderComplieQueue.front());
		MaterialShaderComplieQueue.pop();
	}
}

Shader_NodeGraph * ShaderComplier::GetMaterialShader(MaterialShaderComplieData Data)
{
	auto itor = MaterialShaderMap.find(Data.ToString());
	if (itor != MaterialShaderMap.end())
	{
		return itor->second;
	}

#if 0
	//#AsyncComplie TODO
	EnqeueueMaterialShadercomplie(Data);
	return nullptr;
#else
	return ComplieMateral(Data);
#endif
}

Shader_NodeGraph* ShaderComplier::ComplieMateral(MaterialShaderComplieData data)
{
	Shader_NodeGraph* s = MaterialCompiler->Complie(data);
	ensure(s);
	MaterialShaderMap.emplace(data.ToString(), s);
	return s;
}

void ShaderComplier::EnqeueueMaterialShadercomplie(MaterialShaderComplieData data)
{

}


ShaderType::ShaderType(std::string name, InitliserFunc constructor, ShaderInit & init)
{
	Constructor = constructor;
	ShaderInitalizer = init;
	ShaderComplier::Get()->AddShaderType(name, *this);
}
