#include "Stdafx.h"
#include "ShaderComplier.h"
#include "RHI/Shader.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Engine.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Performance/PerfManager.h"
ShaderComplier * ShaderComplier::Instance = nullptr;
static ConsoleVariable GenDebugShaders("DebugShaders", 0, ECVarType::LaunchOnly);
ShaderComplier::ShaderComplier()
{}

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
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderTypesMap.begin(); it != GlobalShaderTypesMap.end(); ++it)
	{
		ComplieShader(it->second, RHI::GetDefaultDevice());
	}
}

void ShaderComplier::FreeAllGlobalShaders()
{
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderTypesMap.begin(); it != GlobalShaderTypesMap.end(); ++it)
	{
		SafeDelete(it->second.CompliedShader);
	}
}

bool ShaderComplier::ShouldBuildDebugShaders()
{
#if _DEBUG && WITH_EDITOR
	//return !Engine::GetIsCooking();
#endif
	return GenDebugShaders.GetBoolValue();
}

void ShaderComplier::ComplieShader(ShaderType & type, DeviceContext* Context)
{
	type.ShaderInitalizer.Context = Context;
	type.CompliedShader = type.Constructor(type.ShaderInitalizer);
}
//todo: complie materials too
//todo: device index
ShaderType* ShaderComplier::GetShaderFromGlobalMap(std::string name)
{
	StringUtils::RemoveChar(name, "class ");
	if (GlobalShaderTypesMap.find(name) != GlobalShaderTypesMap.end())
	{
		return &GlobalShaderTypesMap.at(name);
	}
	DebugEnsure(false);
	return nullptr;
}

void ShaderComplier::AddShaderType(std::string Name, ShaderType type)
{
	GlobalShaderTypesMap.emplace(Name, type);
}

ShaderType::ShaderType(std::string name, InitliserFunc constructor, ShaderInit & init)
{
	Constructor = constructor;
	ShaderInitalizer = init;
	ShaderComplier::Get()->AddShaderType(name, *this);
}
