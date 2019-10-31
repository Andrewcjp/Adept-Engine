#include "ShaderComplier.h"
#include "Asset_Shader.h"
#include "Core/Engine.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/Shader.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "Rendering/ShaderGraph/ShaderGraphComplier.h"
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "../Platform/Windows/WindowsWindow.h"
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
#if BASIC_RENDER_ONLY
	return;
#endif
	SCOPE_STARTUP_COUNTER("ComplieAllGlobalShaders");
	int CurrnetCount = 0;
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderMapDefinitions.begin(); it != GlobalShaderMapDefinitions.end(); ++it)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			ShaderType NewShader = it->second;
			ComplieShader(NewShader, RHI::GetDeviceContext(i));
			GetShaderMap(RHI::GetDeviceContext(i))->emplace(it->first, NewShader);
		}
		CurrnetCount++;
		PlatformWindow::TickSplashWindow(0, "Loading Global Shaders " + std::to_string(CurrnetCount) + "/" + std::to_string(GetShaderMap()->size()));
	}

}

void ShaderComplier::FreeAllGlobalShaders()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		ShaderMap* Map = GetShaderMap(RHI::GetDeviceContext(i));
		for (std::map<std::string, ShaderType>::iterator it = Map->begin(); it != Map->end(); ++it)
		{
			SafeDelete(it->second.CompliedShader);
		}
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
	if (type.ShouldComplieFunc != nullptr)
	{
		ShaderComplieSettings S;
		S.RTSupported = RHI::GetRenderSettings()->RaytracingEnabled();
		S.ShaderModel = Context->GetCaps().HighestModel;
		if (!type.ShouldComplieFunc(S))
		{
			return;
		}
	}
	if (type.CompliedShader == nullptr)
	{
		type.ShaderInitalizer.Context = Context;
		type.CompliedShader = type.Constructor(type.ShaderInitalizer);
	}
}

ShaderType* ShaderComplier::GetShaderFromGlobalMap(std::string name, DeviceContext* context/* = nullptr*/)
{
	StringUtils::RemoveChar(name, "class ");
	ShaderMap* Map = GetShaderMap(context);
	if (Map->find(name) != Map->end())
	{
		return &Map->at(name);
	}
	DebugEnsure(false);
	return nullptr;
}

void ShaderComplier::AddShaderType(std::string Name, ShaderType type)
{
	GlobalShaderMapDefinitions.emplace(Name, type);
}

#define DEBUG_SLOW_COMPLIE 1
void ShaderComplier::TickMaterialComplie()
{
#if BASIC_RENDER_ONLY
	return;
#endif
#if DEBUG_SLOW_COMPLIE
	if (RHI::GetFrameCount() % 100 != 0)
	{
		return;
	}
#endif
	if (MaterialShaderComplieQueue.empty())
	{
		return;
	}
	for (int i = 0; i < MaterialShaderComplieQueue.size(); i++)
	{
#if DEBUG_SLOW_COMPLIE
		if (i > 0)
		{
			continue;
		}
#endif
		MaterialShaderPair pair = MaterialShaderComplieQueue.front();
		ComplieMaterialShader(pair.Placeholder);
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
	return EnqeueueMaterialShadercomplie(Data);
}

Shader_NodeGraph* ShaderComplier::ComplieMateral(MaterialShaderComplieData data)
{
	Shader_NodeGraph* s = MaterialCompiler->Complie(data);
	ensure(s);
	MaterialShaderMap.emplace(data.ToString(), s);
	return s;
}

void ShaderComplier::ComplieMaterialShader(Shader_NodeGraph* shader)
{
	shader->Init();
}

Shader_NodeGraph* ShaderComplier::EnqeueueMaterialShadercomplie(MaterialShaderComplieData data)
{
	MaterialShaderPair Pair;
	Pair.Data = data;
	Pair.Placeholder = ComplieMateral(data);
	MaterialShaderComplieQueue.emplace(Pair);
	Log::LogMessage("Added Material shader for compile (" + data.Shader->GetName() + ")");
#if 1
	//if (RHI::GetFrameCount() == 0)
	//{
	TickMaterialComplie();
	//}
#endif
	return Pair.Placeholder;
}

ShaderComplier::ShaderMap * ShaderComplier::GetShaderMap(DeviceContext * device)
{
	if (device == nullptr)
	{
		return &GlobalShaderMap[0];
	}
	return &GlobalShaderMap[device->GetDeviceIndex()];
}


ShaderType::ShaderType(std::string name, InitliserFunc constructor, const ShaderInit & init, ShouldComplieSig func)
{
	Constructor = constructor;
	ShaderInitalizer = init;
	ShouldComplieFunc = func;
	ShaderComplier::Get()->AddShaderType(name, *this);
}
