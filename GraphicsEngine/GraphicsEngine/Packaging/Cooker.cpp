#include "stdafx.h"
#include "Cooker.h"
#include "../Core/Utils/WindowsHelper.h"
#include <experimental/filesystem>
#include "../Core/Engine.h"
#include "../Core/Utils/FileUtils.h"
#include <map>
#include "../Core/Assets/AssetManager.h"
Cooker::Cooker()
{
}
Cooker::Cooker(AssetManager* ASM)
{
	AssetM = ASM;
}


Cooker::~Cooker()
{
}
//returns the root path of the output dir
std::string Cooker::GetTargetPath(bool AppendSlash)
{
	std::string TargetPath = Engine::GetRootDir();
	StringUtils::RemoveChar(TargetPath, "\\x64");
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
	std::cout << "**********Cook Started**********" << std::endl;
	if (!FileUtils::exists_test3(GetTargetPath()))
	{
		(!FileUtils::TryCreateDirectory(GetTargetPath()));
	}
	std::string path = Engine::GetRootDir();
	path.append("\\Release\\");
	//Copy Binaries 
	int SumSize = 0;
	for (auto & p : std::experimental::filesystem::directory_iterator(path))
	{
		std::string destpath = p.path().string();
		StringUtils::RemoveChar(destpath, path);
		if (destpath.find(".dll") != -1 || destpath.find(".exe") != -1)
		{
			SumSize += fs::file_size(p);
			std::string out = GetTargetPath(true);
			out.append(destpath);
			WindowsHelpers::CopyFileToTarget(p.path().string(), out);
		}
	}
	std::cout << "Copied " << SumSize / 1e6 << "mb of Binaries to output" << std::endl;
	//copy assets
	if (AssetM != nullptr)
	{
		for (std::map<std::string, TextureAsset>::iterator it = AssetM->GetTextureMap()->begin(); it != AssetM->GetTextureMap()->end(); ++it)
		{
			CopyAssetToOutput(it->first);
		}
		for (std::map<std::string, std::string>::iterator it = AssetM->GetMeshMap()->begin(); it != AssetM->GetMeshMap()->end(); ++it)
		{
			CopyAssetToOutput(it->first);
		}
		//copy Cooked Shaders
		CopyAssetToOutput(AssetM->ShaderCookedFile);
		//copy font
		CopyAssetToOutput("\\asset\\fonts\\arial.ttf");
	}
	std::cout << "**********Cook Complete**********" << std::endl;
	 
}
bool Cooker::CopyAssetToOutput(std::string RelTarget)
{
	std::string TargetDir = GetTargetPath();
	TargetDir.append(RelTarget);
	std::string Targetfile = TargetDir;
	int pos = TargetDir.find_last_of('\\');
	TargetDir.erase(TargetDir.begin() + pos, TargetDir.end());

	if (!FileUtils::exists_test3(TargetDir))
	{
		if (!FileUtils::CreateDirectoryFromFullPath(GetTargetPath(), TargetDir, false))
		{
			/*return false;*/
		}
	}
	std::string source = Engine::GetRootDir();
	source.append(RelTarget);
	if (!WindowsHelpers::CopyFileToTarget(source, Targetfile))
	{
		return false;
	}
	return true;
}
void Cooker::CreatePackage()
{

}

