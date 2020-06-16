
#include "StandaloneShaderCompilerEntry.h"
#include "Core/Assets/ShaderCompiler.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Performance/PerfManager.h"
#include "Packaging/Cooker.h"

StandaloneShaderCompilerEntry::StandaloneShaderCompilerEntry(int argc, char *argv[])
{

	PerfManager::Get()->StartSingleActionTimer("Shader Compiler Run");
	std::string CMDLine;
	std::vector<std::string> Args;
	for (int i = 0; i < argc; i++)
	{
		Args.push_back(argv[i]);
		if (i > 0)
		{
			CMDLine += argv[i];
		}
	}
	EPlatforms::Type TargetPlatform = EPlatforms::Windows;
	EShaderSupportModel::Type TargetSM = EShaderSupportModel::SM6;
	for (int i = 0; i < Args.size(); i++)
	{
		StringUtils::ToLower(Args[i]);
		EPlatforms::Type parsed = EPlatforms::Parse(Args[i]);
		if (parsed != EPlatforms::Limit)
		{
			TargetPlatform = parsed;
			continue;
		}
		if (Args[i].find("sm5") != std::string::npos)
		{
			TargetSM = EShaderSupportModel::SM5;
		}
		if (Args[i].find("sm6") != std::string::npos)
		{
			TargetSM = EShaderSupportModel::SM6;
		}
	}

	AssetManager::StartAssetManager();

	ShaderCompiler::Get();
	RHI::InitRHI(ERenderSystemType::RenderSystemNull);
	//RHI::InitialiseContext();
	RHI::GetRHIClass()->InitRHI(false);
	ShaderCompiler::Get()->Init();
	ShaderCompiler::Get()->m_Config.MirrorToOthers = false;
	ShaderCompiler::Get()->m_Config.ShaderModelTarget = TargetSM;
	ShaderCompiler::Get()->m_Config.TargetPlatform = TargetPlatform;
	Log::StartLogger();
	Log::LogMessage("Started shader Compiler with cmd line '" + CMDLine + "'");
}

StandaloneShaderCompilerEntry::~StandaloneShaderCompilerEntry()
{
	PerfManager::Get()->EndAndLogTimer("Shader Compiler Run");
	ShaderCompiler::ShutDown();
	Log::ShutDownLogger();
}

int StandaloneShaderCompilerEntry::Execute()
{
	ShaderCompiler::Get()->CompileAllGlobalShaders();
	Cooker::BuildAllMaterials();
	return 0;
}
