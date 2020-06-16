#include "ShaderCompiler.h"
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
#include "Rendering/ShaderGraph/ShaderGraphCompiler.h"
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "RHI/Shader.h"
#include "RHI/ShaderCompilerModule.h"
#include "Rendering/Shaders/GlobalShaderLibrary.h"

ShaderCompiler * ShaderCompiler::Instance = nullptr;
static ConsoleVariable GenDebugShaders("DebugShaders", 0, ECVarType::LaunchOnly);

ShaderCompiler::ShaderCompiler()
{
	MaterialCompiler = new ShaderGraphCompiler();

}

ShaderCompiler::~ShaderCompiler()
{}

ShaderCompiler * ShaderCompiler::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ShaderCompiler();
	}
	return Instance;
}

void ShaderCompiler::ShutDown()
{
	SafeDelete(Instance);
}

void ShaderCompiler::CompileAllGlobalShaders()
{
	SCOPE_STARTUP_COUNTER("CompileAllGlobalShaders");
	int CurrentCount = 0;
	GlobalShaderLibrary::Init();
	for (std::map<std::string, ShaderType>::iterator it = GlobalShaderMapDefinitions.begin(); it != GlobalShaderMapDefinitions.end(); ++it)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			ShaderType NewShader = it->second;
			CompileShader(NewShader, RHI::GetDeviceContext(i));
			GetShaderMap(RHI::GetDeviceContext(i))->emplace(it->first, NewShader);
		}
		CurrentCount++;
		PlatformWindow::TickSplashWindow(0, "Loading Global Shaders " + std::to_string(CurrentCount) + "/" + std::to_string(GlobalShaderMapDefinitions.size() + SingleShaderMapDefinitions.size()));
	}

	for (auto it = SingleShaderMapDefinitions.begin(); it != SingleShaderMapDefinitions.end(); ++it)
	{
		ShaderCompileItem Item;
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

void ShaderCompiler::FreeAllGlobalShaders()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		ShaderMap* Map = GetShaderMap(RHI::GetDeviceContext(i));
		for (std::map<std::string, ShaderType>::iterator it = Map->begin(); it != Map->end(); ++it)
		{
			SafeDelete(it->second.CompiledShader);
		}
	}
}

bool ShaderCompiler::ShouldBuildDebugShaders()
{
	return GenDebugShaders.GetBoolValue();
}

void ShaderCompiler::CompileShader(ShaderType & type, DeviceContext* Context)
{
	if (type.ShouldCompileFunc != nullptr)
	{
		ShaderCompileSettings S;
		S.RTSupported = RHI::GetRenderSettings()->RaytracingEnabled();
		S.ShaderModel = m_Config.ShaderModelTarget;
		if (!type.ShouldCompileFunc(S))
		{
			return;
		}
	}
	if (type.CompiledShader == nullptr)
	{
		type.ShaderInitalizer.Context = Context;
		type.CompiledShader = type.Constructor(type.ShaderInitalizer);
	}
}

ShaderType* ShaderCompiler::GetShaderFromGlobalMap(std::string name, DeviceContext* context/* = nullptr*/)
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

void ShaderCompiler::AddShaderType(std::string Name, ShaderType type)
{
	GlobalShaderMapDefinitions.emplace(Name, type);
}

#define DEBUG_SLOW_COMPILE 1
void ShaderCompiler::TickMaterialCompile()
{
#if DEBUG_SLOW_COMPILE
	if (RHI::GetFrameCount() % 100 != 0)
	{
		return;
	}
#endif
	if (MaterialShaderCompileQueue.empty())
	{
		return;
	}
	for (int i = 0; i < MaterialShaderCompileQueue.size(); i++)
	{
#if DEBUG_SLOW_COMPILE
		if (i > 0)
		{
			continue;
		}
#endif
		MaterialShaderPair pair = MaterialShaderCompileQueue.front();
		CompileMaterialShader(pair.Placeholder);
		MaterialShaderCompileQueue.pop();
	}
}

Shader_NodeGraph * ShaderCompiler::GetMaterialShader(MaterialShaderCompileData Data)
{
	auto itor = MaterialShaderMap.find(Data.ToString());
	if (itor != MaterialShaderMap.end())
	{
		return itor->second;
	}
	return EnqeueueMaterialShadercompile(Data);
}

Shader_NodeGraph* ShaderCompiler::CompileMateral(MaterialShaderCompileData data)
{
	Shader_NodeGraph* s = MaterialCompiler->Compile(data);
	ensure(s);
	MaterialShaderMap.emplace(data.ToString(), s);
	return s;
}

void ShaderCompiler::CompileMaterialShader(Shader_NodeGraph* shader)
{
	shader->Init();
}

Shader_NodeGraph* ShaderCompiler::EnqeueueMaterialShadercompile(MaterialShaderCompileData data)
{
	MaterialShaderPair Pair;
	Pair.Data = data;
	Pair.Placeholder = CompileMateral(data);
	MaterialShaderCompileQueue.emplace(Pair);
	Log::LogMessage("Added Material shader for compile (" + data.Shader->GetName() + ")");
#if 1
	//if (RHI::GetFrameCount() == 0)
	//{
	TickMaterialCompile();
	//}
#endif
	return Pair.Placeholder;
}

void ShaderCompiler::RegisterShaderCompiler(std::string DLLName)
{
	ShaderCompilerNames.push_back(DLLName);
}

void ShaderCompiler::Init()
{
	m_Config.TargetPlatform = PlatformApplication::GetPlatform();
	m_Config.ShaderModelTarget = RHI::GetDefaultDevice()->GetCaps().HighestModel;
	m_Config.MirrorToOthers = m_Config.TargetPlatform == EPlatforms::Windows;
#if defined(PLATFORM_WINDOWS) && WITH_EDITOR
	ShaderCompilerNames.push_back("WindowsShaderCompiler");
	ShaderCompilerNames.push_back("WindowsLegacyShaderCompiler");
	FindAndLoadCompilers();
#endif
}

void ShaderCompiler::FindAndLoadCompilers()
{
	for (int i = 0; i < ShaderCompilerNames.size(); i++)
	{
		std::string CompilerName = ShaderCompilerNames[i];
		IShaderCompiler* Compiler = ModuleManager::Get()->GetModule<IShaderCompiler>(FString(CompilerName));
		if (Compiler != nullptr)
		{
			ShaderCompilers.push_back(Compiler);
		}
	}
	PlatformInterface::RegisterShaderCompilers(ShaderCompilers);
}

void ShaderCompiler::CompileShaderNew(ShaderCompileItem* Item, EPlatforms::Type platform)
{
	Item->TargetPlatfrom = platform;
	Item->CompileShaderDebug = ShaderCompiler::Get()->ShouldBuildDebugShaders();
	for (int i = 0; i < ShaderCompilers.size(); i++)
	{
		if (ShaderCompilers[i]->SupportsPlatform(platform, Item->ShaderModel, Item))
		{
			ShaderCompilers[i]->CompileShader(Item);
			return;
		}
	}
	//AD_Assert_Always("No shader compiler found for shader");
}

void ShaderCompiler::RegisterSingleShader(SingleShaderCompile * Target)
{
	SingleShaderMapDefinitions.emplace(Target->GetHash(), Target);
}

bool ShaderCompiler::CheckSourceFileRegistered(std::string file)
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


ShaderCompiler::ShaderMap * ShaderCompiler::GetShaderMap(DeviceContext * device)
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
	ShouldCompileFunc = func;
	ShaderCompiler::Get()->AddShaderType(name, *this);
}

SingleShaderCompile::SingleShaderCompile(std::string name, std::string file, ShaderProgramBase::Shader_Define Define, EShaderType::Type stage)
{
	Name = name;
	SourceFile = file;
	Defines.push_back(Define);
	Stage = stage;
	ShaderCompiler::Get()->RegisterSingleShader(this);
}

SingleShaderCompile::SingleShaderCompile(std::string name, std::string file, EShaderType::Type stage)
{
	Name = name;
	SourceFile = file;
	Stage = stage;
	ShaderCompiler::Get()->RegisterSingleShader(this);
}

uint64 SingleShaderCompile::GetHash()
{
	uint64 hash = std::hash<std::string>{} (SourceFile);
	return hash;
}
