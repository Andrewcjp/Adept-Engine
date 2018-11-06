
#include "Cooker.h"
#include <experimental/filesystem>
#include "Core/Engine.h"
#include "Core/Utils/FileUtils.h"
#include <map>
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Assets/ShaderComplier.h"
#include "RHI/RHI.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
static ConsoleVariable CookDebug("CookDebug", 0, ECVarType::LaunchOnly);
Cooker::Cooker()
{
	bool BuildDebug = CookDebug.GetBoolValue();
#if _DEBUG
	BuildDebug = false;
#endif
	BuildConfig = "ShippingReleasePackage";
	if (BuildDebug)
	{
		BuildConfig = "ShippingDebugPackage";
		Log::LogMessage("Cooking ShippingDebugPackage");
	}
	OutputPath += "\\" + BuildConfig;
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
namespace fs = std::experimental::filesystem;
void Cooker::CopyToOutput()
{
	if (ShouldComplie)
	{
		Log::OutS << "**********Complie Started**********" << Log::OutS;
		if (PlatformApplication::ExecuteHostScript(AssetManager::GetScriptPath() + "\\BuildSLN.bat", BuildConfig, true) != 0)
		{
			Log::LogMessage("Complie Failed, Aborting Cook", Log::Severity::Error);
			::Sleep(2000);
			Engine::Exit(-1);
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
		for (auto & p : std::experimental::filesystem::directory_iterator(path))
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
		//CopyFolderToOutput(AssetManager::GetContentPath(), "\\Content\\");
		CopyFolderToOutput(AssetManager::GetContentPath() + "models", "\\Content\\models");
		CopyFolderToOutput(AssetManager::GetDDCPath(), "\\DerivedDataCache\\");
		//copy font
		CopyAssetToOutput("\\Content\\fonts\\arial.ttf");
		CopyAssetToOutput("\\DerivedDataCache\\T_GridSmall_01_D.DDS");
		//temp
		CopyAssetToOutput("\\Content\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds");
		CopyAssetToOutput("\\Content\\texture\\house_diffuse.tga");
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
	//ShaderComplier::Get()->ComplieAllGlobalShaders();
}

void Cooker::CopyFolderToOutput(std::string Target, std::string PathFromBuild)
{
	uintmax_t SumSize = 0;
	FileUtils::CreateDirectoriesToFullPath(GetTargetPath() + PathFromBuild);
	for (auto & p : std::experimental::filesystem::recursive_directory_iterator(Target))
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

