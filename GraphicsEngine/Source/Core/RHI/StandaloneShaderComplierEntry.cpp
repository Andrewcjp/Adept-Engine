
#include "StandaloneShaderComplierEntry.h"
#include "Core/Assets/ShaderComplier.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Performance/PerfManager.h"

StandaloneShaderComplierEntry::StandaloneShaderComplierEntry(int argc, char *argv[])
{

	PerfManager::Get()->StartSingleActionTimer("Shader Complier Run");
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

	ShaderComplier::Get();
	RHI::InitRHI(ERenderSystemType::RenderSystemNull);
	//RHI::InitialiseContext();
	RHI::GetRHIClass()->InitRHI(false);
	ShaderComplier::Get()->Init();
	ShaderComplier::Get()->m_Config.MirrorToOthers = false;
	ShaderComplier::Get()->m_Config.ShaderModelTarget = TargetSM;
	ShaderComplier::Get()->m_Config.TargetPlatform = TargetPlatform;
	Log::StartLogger();
	Log::LogMessage("Started shader complier with cmd line '" + CMDLine + "'");
}

StandaloneShaderComplierEntry::~StandaloneShaderComplierEntry()
{
	PerfManager::Get()->EndAndLogTimer("Shader Complier Run");
	ShaderComplier::ShutDown();
	Log::ShutDownLogger();
}

int StandaloneShaderComplierEntry::Execute()
{
	ShaderComplier::Get()->ComplieAllGlobalShaders();
	return 0;
}
