#include "stdafx.h"
#include "Cooker.h"
#include <experimental/filesystem>
#include "Core/Engine.h"
#include "Core/Utils/FileUtils.h"
#include <map>
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/ConsoleVariable.h"
Cooker::Cooker()
{}
Cooker::Cooker(AssetManager* ASM)
{
	AssetM = ASM;
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
static ConsoleVariable CookDebug("CookDebug", 0, ECVarType::LaunchOnly);
void Cooker::CopyToOutput()
{
	bool BuildDebug = CookDebug.GetBoolValue();
#if _DEBUG
	BuildDebug = true;
#endif
	std::string BuildConfig = "ShippingReleasePackage";
	if (BuildDebug)
	{
		BuildConfig = "ShippingDebugPackage";
	}
	Log::OutS << "**********Complie Started**********" << Log::OutS;
	if (PlatformApplication::ExecuteHostScript(AssetManager::GetScriptPath() + "\\BuildSLN.bat", BuildConfig, true) != 0)
	{
		Log::LogMessage("Complie Failed, Aborting Cook", Log::Severity::Error);
		Sleep(2000);
		Engine::Exit(-1);
	}
	Log::OutS << "**********Complie Finished**********" << Log::OutS;
	Log::OutS << "**********Cook Started**********" << Log::OutS;
	if (!FileUtils::File_ExistsTest(GetTargetPath()))
	{
		(!PlatformApplication::TryCreateDirectory(GetTargetPath()));
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
	if (AssetM != nullptr)
	{
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
		CopyFolderToOutput(AssetManager::GetContentPath()+"models", "\\Content\\models");
		CopyFolderToOutput(AssetManager::GetDDCPath(), "\\DerivedDataCache\\");
		//copy Cooked Shaders
		//CopyAssetToOutput(AssetM->ShaderCookedFile);
		//copy font
		CopyAssetToOutput("\\Content\\fonts\\arial.ttf");
		CopyAssetToOutput("\\DerivedDataCache\\T_GridSmall_01_D.DDS");
		//temp
		CopyAssetToOutput("\\Content\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds");

		Log::LogMessage("Copied " + std::to_string(contentItemCount) + " Items");
	}
	Log::OutS << "**********Cook Complete**********" << Log::OutS;

}

void Cooker::CopyFolderToOutput(std::string Target, std::string PathFromBuild)
{
	uintmax_t SumSize = 0;
	FileUtils::CreateDirectoriesToFullPath(GetTargetPath()+ PathFromBuild);
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

}

