#include "stdafx.h"
#include "Cooker.h"
#include <experimental/filesystem>
#include "Core/Engine.h"
#include "Core/Utils/FileUtils.h"
#include <map>
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/PlatformCore.h"
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
void Cooker::CopyToOutput()
{
	Log::OutS << "**********Cook Started**********" << Log::OutS;
	if (!FileUtils::File_ExistsTest(GetTargetPath()))
	{
		(!PlatformApplication::TryCreateDirectory(GetTargetPath()));
	}
	std::string path = Engine::GetExecutionDir();
	path.append("\\Release\\");
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
		CopyFolderToOutput(AssetManager::GetContentPath(), "\\Content\\");
		//copy Cooked Shaders
		//CopyAssetToOutput(AssetM->ShaderCookedFile);
		//copy font
		CopyAssetToOutput("\\Content\\fonts\\arial.ttf");
		CopyAssetToOutput("\\DerivedDataCache\\T_GridSmall_01_D.DDS");
		Log::LogMessage("Copied " + std::to_string(contentItemCount) + " Items");
	}
	Log::OutS << "**********Cook Complete**********" << Log::OutS;

}

void Cooker::CopyFolderToOutput(std::string Target, std::string PathFromBuild)
{
	uintmax_t SumSize = 0;
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
			PlatformApplication::CopyFileToTarget(p.path().string(), out);
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

