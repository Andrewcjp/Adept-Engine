#include "ShaderComplier.h"
#include "Asset types/Asset_Shader.h"
#include "AssetManager.h"
#include "Core/Engine.h"
#include "Core/Module/ModuleManager.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "Core/Utils/StringUtil.h"
#include "Packaging/Cooker.h"
#include "Rendering/Core/ShaderCache.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "Rendering/ShaderGraph/ShaderGraphComplier.h"
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "RHI/Shader.h"
#include "RHI/ShaderComplierModule.h"
#include "Rendering/Shaders/GlobalShaderLibrary.h"

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

void ShaderComplier::ShutDown()
{
	SafeDelete(Instance);
}

void ShaderComplier::ComplieAllGlobalShaders()
{
	SCOPE_STARTUP_COUNTER("ComplieAllGlobalShaders");
	int CurrentCount = 0;
	GlobalShaderLibrary::Init();
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderMapDefinitions.begin(); it != GlobalShaderMapDefinitions.end(); ++it)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			ShaderType NewShader = it->second;
			ComplieShader(NewShader, RHI::GetDeviceContext(i));
			GetShaderMap(RHI::GetDeviceContext(i))->emplace(it->first, NewShader);
		}
		CurrentCount++;
		PlatformWindow::TickSplashWindow(0, "Loading Global Shaders " + std::to_string(CurrentCount) + "/" + std::to_string(GlobalShaderMapDefinitions.size() + SingleShaderMapDefinitions.size()));
	}

	for (auto it = SingleShaderMapDefinitions.begin(); it != SingleShaderMapDefinitions.end(); ++it)
	{
		ShaderComplieItem Item;
		Item.Data = AssetManager::Get()->LoadFileWithInclude(it->second->SourceFile + ".hlsl");
		Item.Defines = it->second->Defines;
		ShaderProgramBase::AddDefaultDefines(Item.Defines, RHI::GetDeviceContext(0));
		Item.Stage = it->second->Stage;
		Item.EntryPoint = "main";
		Item.ShaderName = it->second->SourceFile;
		Item.TargetPlatfrom = m_Config.TargetPlatform;
		Item.ShaderModel = m_Config.ShaderModelTarget;
		ShaderCache::Get()->GetShader(&Item);
		CurrentCount++;
		PlatformWindow::TickSplashWindow(0, "Loading Global Shaders " + std::to_string(CurrentCount) + "/" + std::to_string(GlobalShaderMapDefinitions.size() + SingleShaderMapDefinitions.size()));
	}
	ShaderCache::Get()->PrintShaderStats();
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
	return GenDebugShaders.GetBoolValue();
}

void ShaderComplier::ComplieShader(ShaderType & type, DeviceContext* Context)
{
	if (type.ShouldComplieFunc != nullptr)
	{
		ShaderComplieSettings S;
		S.RTSupported = RHI::GetRenderSettings()->RaytracingEnabled();
		S.ShaderModel = m_Config.ShaderModelTarget;
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

void ShaderComplier::RegisterShaderComplier(std::string DLLName)
{
	ShaderComplierNames.push_back(DLLName);
}

void ShaderComplier::Init()
{
	m_Config.TargetPlatform = PlatformApplication::GetPlatform();
	m_Config.ShaderModelTarget = RHI::GetDefaultDevice()->GetCaps().HighestModel;
	m_Config.MirrorToOthers = m_Config.TargetPlatform == EPlatforms::Windows;
#if defined(PLATFORM_WINDOWS) && WITH_EDITOR
	ShaderComplierNames.push_back("WindowsShaderCompiler");
	ShaderComplierNames.push_back("WindowsLegacyShaderComplier");
	FindAndLoadCompliers();
#endif
}

void ShaderComplier::FindAndLoadCompliers()
{
	for (int i = 0; i < ShaderComplierNames.size(); i++)
	{
		std::string ComplierName = ShaderComplierNames[i];
		IShaderComplier* Complier = ModuleManager::Get()->GetModule<IShaderComplier>(FString(ComplierName));
		if (Complier != nullptr)
		{
			Shadercompliers.push_back(Complier);
		}
	}
	PlatformInterface::RegisterShaderCompliers(Shadercompliers);
}

void ShaderComplier::ComplieShaderNew(ShaderComplieItem* Item, EPlatforms::Type platform)
{
	Item->TargetPlatfrom = platform;
	Item->ComplieShaderDebug = ShaderComplier::Get()->ShouldBuildDebugShaders();
	for (int i = 0; i < Shadercompliers.size(); i++)
	{
		if (Shadercompliers[i]->SupportsPlatform(platform, Item->ShaderModel, Item))
		{
			Shadercompliers[i]->ComplieShader(Item);
			return;
		}
	}
	//AD_Assert_Always("No shader complier found for shader");
}

void ShaderComplier::RegisterSingleShader(SingleShaderComplie * Target)
{
	SingleShaderMapDefinitions.emplace(Target->GetHash(), Target);
}

bool ShaderComplier::CheckSourceFileRegistered(std::string file)
{
	for (auto it = SingleShaderMapDefinitions.begin(); it != SingleShaderMapDefinitions.end(); ++it)
	{
		if (it->second->SourceFile == file)
		{
			return true;
		}
	}
	return false;
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

SingleShaderComplie::SingleShaderComplie(std::string name, std::string file, ShaderProgramBase::Shader_Define Define, EShaderType::Type stage)
{
	Name = name;
	SourceFile = file;
	Defines.push_back(Define);
	Stage = stage;
	ShaderComplier::Get()->RegisterSingleShader(this);
}

SingleShaderComplie::SingleShaderComplie(std::string name, std::string file, EShaderType::Type stage)
{
	Name = name;
	SourceFile = file;
	Stage = stage;
	ShaderComplier::Get()->RegisterSingleShader(this);
}

uint64 SingleShaderComplie::GetHash()
{
	uint64 hash = std::hash<std::string>{} (SourceFile);
	return hash;
}
