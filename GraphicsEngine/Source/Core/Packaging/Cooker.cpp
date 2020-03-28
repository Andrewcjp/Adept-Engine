#include "Cooker.h"
#if SUPPORTS_COOK
#include <filesystem>
#include "Core/Utils/FileUtils.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "Rendering/Core/Mesh/MaterialTypes.h"
#include "Rendering/Core/Material.h"
#include "Core/Assets/ShaderComplier.h"
#include "../Core/Rendering/Shaders/Raytracing/Shader_RTMateralHit.h"
#include "../Core/Core/Game/Game.h"
static ConsoleVariable CookDebug("CookDebug", 0, ECVarType::LaunchOnly);
static ConsoleVariable CookPack("pack", 0, ECVarType::LaunchOnly);
Cooker::Cooker()
{
	bool BuildDebug = CookDebug.GetBoolValue();

	ShouldComplie = false; 
	ShouldPack = CookPack.GetBoolValue();
	BuildConfig = "ReleasePackage";
	if (BuildDebug)
	{
		BuildConfig = "ShippingDebugPackage";
		Log::LogMessage("Cooking ShippingDebugPackage");
	}
	SetPlatform(PlatformApplication::GetPlatform());
}

Cooker::~Cooker()
{}
//returns the root path of the output dir
std::string Cooker::GetTargetPath(bool AppendSlash)
{
	std::string TargetPath = AssetManager::GetRootDir();
	TargetPath.append(OutputPath);
	if (AppendSlash)
	{
		TargetPath.append("\\");
	}
	return TargetPath;
}
namespace fs = std::filesystem;
void Cooker::Execute()
{
#include "Core/Engine.h"
#include "Core/Game/Game.h"
#include "Rendering/Shaders/Raytracing/Shader_RTMateralHit.h"
	Log::LogMessage("Cooking for Platform: " + EPlatforms::ToString(TargetPlatform));
	if (ShouldComplie)
	{
		Log::OutS << "**********Complie Started**********" << Log::OutS;
		if (PlatformApplication::ExecuteHostScript(AssetManager::GetScriptPath() + "\\BuildSLN.bat", BuildConfig, true) != 0)
		{
			Log::LogMessage("Complie Failed, Aborting Cook", Log::Severity::Error);
			::Sleep(2000);
			Engine::RequestExit(-1);
		}
		Log::OutS << "**********Complie Finished**********" << Log::OutS;
	}
	Log::OutS << "**********Cook Started**********" << Log::OutS;
	CookAllShaders();
	Log::OutS << "**********Cook Complete**********" << Log::OutS;
	if (ShouldStage)
	{
		Log::OutS << "**********Stage Start**********" << Log::OutS;
		if (!FileUtils::File_ExistsTest(GetTargetPath()))
		{
			FileUtils::CreateDirectoriesToFullPath(GetTargetPath());
		}
		std::string path = Engine::GetExecutionDir();
		path.append("\\" + BuildConfig + "\\");

		//Copy Binaries 
		uintmax_t SumSize = 0;
		for (auto & p : std::filesystem::directory_iterator(path))
		{
			std::string destpath = p.path().string();
			StringUtils::RemoveChar(destpath, path);
			if (destpath.find(".dll") != -1 || destpath.find(".exe") != -1)
			{
				SumSize += fs::file_size(p);
				std::string out = GetTargetPath(true);
				out.append(destpath);
				PlatformApplication::CopyFileToTarget(p.path().string(), out);
			}
		}
		Log::OutS << "Copied " << SumSize / 1e6 << "mb of Binaries to output" << Log::OutS;
		//copy assets
		int contentItemCount = 0;

		/*for (std::map<std::string, TextureAsset>::iterator it = AssetM->GetTextureMap()->begin(); it != AssetM->GetTextureMap()->end(); ++it)
		{
			contentItemCount++;
			CopyAssetToOutput(it->first);
		}
		for (std::map<std::string, std::string>::iterator it = AssetM->GetMeshMap()->begin(); it != AssetM->GetMeshMap()->end(); ++it)
		{
			contentItemCount++;
			CopyAssetToOutput(it->first);
		}*/
#if 0
		CopyFolderToOutput(AssetManager::GetContentPath(), "\\Content\\");
#else
		CopyFolderToOutput(AssetManager::GetContentPath() + "models", "\\Content\\models");
		CopyFolderToOutput(AssetManager::GetContentPath() + "AlwaysCook", "\\Content\\AlwaysCook");
#endif
		CopyFolderToOutput(AssetManager::GetDDCPath(), "\\DerivedDataCache\\");
		//copy font
		//CopyAssetToOutput("\\Content\\fonts\\arial.ttf");
		CopyAssetToOutput("\\DerivedDataCache\\T_GridSmall_01_D.DDS");
		//temp
		CopyAssetToOutput("\\Content\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds");
		//CopyAssetToOutput("\\Content\\texture\\house_diffuse.tga");
		Log::LogMessage("Copied " + std::to_string(contentItemCount) + " Items");

		Log::OutS << "**********Stage Complete**********" << Log::OutS;
	}
	if (ShouldPack)
	{
		Log::OutS << "**********Packing Start**********" << Log::OutS;
		CreatePackage();
		Log::OutS << "**********Packing Complete**********" << Log::OutS;
	}
}

void Cooker::CookAllShaders()
{
	ShaderComplier::Get()->ComplieAllGlobalShaders();
	BuildAllMaterials();
	std::vector<std::string> PreLoadTextures = Engine::GetGame()->GetPreLoadAssets();
	for (int i = 0; i < PreLoadTextures.size(); i++)
	{
		AssetManager::DirectLoadTextureAsset(PreLoadTextures[i]);
	}
}

void Cooker::CopyFolderToOutput(std::string Target, std::string PathFromBuild)
{
	uintmax_t SumSize = 0;
	FileUtils::CreateDirectoriesToFullPath(GetTargetPath() + PathFromBuild);
	for (auto & p : std::filesystem::recursive_directory_iterator(Target))
	{
		std::string destpath = p.path().string();
		StringUtils::RemoveChar(destpath, Target);
		/*if (destpath.find(".dll") != -1 || destpath.find(".exe") != -1)
		{*/
		std::string out = GetTargetPath(true);
		destpath = PathFromBuild + destpath;
		out.append(destpath);
		if (!fs::is_directory(p))
		{
			SumSize += fs::file_size(p);
			ensure(PlatformApplication::CopyFileToTarget(p.path().string(), out));
		}
		else
		{
			FileUtils::CreateDirectoryFromFullPath(GetTargetPath(), destpath, false);
		}
	}
	Log::LogMessage("Copied " + std::to_string(SumSize / 1e6) + "Mb Data");
}

bool Cooker::CopyAssetToOutput(std::string RelTarget)
{
	std::string TargetDir = GetTargetPath();
	TargetDir.append(RelTarget);
	std::string Targetfile = TargetDir;
	size_t pos = TargetDir.find_last_of('\\');
	TargetDir.erase(TargetDir.begin() + pos, TargetDir.end());

	if (!FileUtils::File_ExistsTest(TargetDir))
	{
		if (!FileUtils::CreateDirectoryFromFullPath(GetTargetPath(), TargetDir, false))
		{
			/*return false;*/
		}
	}
	std::string source = AssetManager::GetRootDir();
	source.append(RelTarget);
	if (!PlatformApplication::CopyFileToTarget(source, Targetfile))
	{
		return false;
	}
	return true;
}

void Cooker::CreatePackage()
{
	bool foundWindRar = false;
	std::string WinRarInstallDir = "C:\\Program Files\\WinRAR\\WinRAR.exe";
	if (FileUtils::File_ExistsTest(WinRarInstallDir))
	{
		foundWindRar = true;
	}

	if (foundWindRar)
	{
		std::string PackageOutput = AssetManager::GetRootDir() + "\\Packed\\";
		FileUtils::CreateDirectoriesToFullPath(PackageOutput);
		std::string Args = " a -ep1 -r " + PackageOutput + BuildConfig + ".zip " + GetTargetPath() + "\\*.*";

		PlatformApplication::ExecuteHostScript(WinRarInstallDir, Args, GetTargetPath() + "\\", true);
	}

}

void Cooker::BuildAllMaterials()
{
	MaterialShaderComplieData ComplieData;
	ComplieData.Shader = Material::CreateDefaultMaterialInstance()->GetShaderAsset();
	ComplieData.RenderPassUsage = EMaterialPassType::Deferred;
	ComplieData.ShaderKeyWords.clear();
	ShaderComplier::Get()->GetMaterialShader(ComplieData);
	ComplieData.ShaderKeyWords.push_back(Material::ShadowShaderstring);
	ShaderComplier::Get()->GetMaterialShader(ComplieData);

	ComplieData.ShaderKeyWords.clear();
	ComplieData.RenderPassUsage = EMaterialPassType::Forward;
	ShaderComplier::Get()->GetMaterialShader(ComplieData);
	ComplieData.ShaderKeyWords.push_back(Material::ShadowShaderstring);
	ShaderComplier::Get()->GetMaterialShader(ComplieData);
	ComplieData.ShaderKeyWords.clear();
	ComplieData.MaterialRenderType = EMaterialRenderType::Transparent;
	ShaderComplier::Get()->GetMaterialShader(ComplieData);
	ComplieData.ShaderKeyWords.push_back(Material::ShadowShaderstring);
	ShaderComplier::Get()->GetMaterialShader(ComplieData);

	ShaderComplier::Get()->TickMaterialComplie();
	new Shader_RTMateralHit(RHI::GetDefaultDevice());
	new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultAnyHit", ERTShaderType::AnyHit);
}

void Cooker::BuildAll()
{

}

void Cooker::SetPlatform(EPlatforms::Type Platform)
{
	TargetPlatform = Platform;
	OutputPath = "\\Build\\" + EPlatforms::ToString(TargetPlatform) + "\\" + BuildConfig;
}

EPlatforms::Type Cooker::GetTargetPlatform() const
{
	return TargetPlatform;
}

ERenderSystemType Cooker::GetTargetRHI() const
{
	switch (TargetPlatform)
	{
		case EPlatforms::Windows_DX12:
		case EPlatforms::Windows:
			return ERenderSystemType::RenderSystemD3D12;
		case EPlatforms::Windows_VK:
		case EPlatforms::Linux:
		case EPlatforms::Android:
			return ERenderSystemType::RenderSystemVulkan;
	}
	return ERenderSystemType::RenderSystemD3D12;
}

#endif