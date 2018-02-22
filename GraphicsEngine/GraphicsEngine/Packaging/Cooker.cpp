#include "stdafx.h"
#include "Cooker.h"
#include "../Core/Utils/WindowsHelper.h"
#include <experimental/filesystem>
#include "../Core/Engine.h"
#include "../Core/Utils/FileUtils.h"
Cooker::Cooker()
{
}


Cooker::~Cooker()
{
}
namespace fs = std::experimental::filesystem;
void Cooker::CopyToOutput()
{
	std::cout << "**********Cook Started**********" << std::endl;
	std::string TargetPath = Engine::GetRootDir();
	StringUtils::RemoveChar(TargetPath, "\\x64");
	TargetPath.append(OutputPath);
	if (!FileUtils::exists_test3(TargetPath))
	{
		FileUtils::TryCreateDirectory(TargetPath);
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
			std::string out = TargetPath;
			out.append(destpath);
			WindowsHelpers::CopyFileToTarget(p.path().string(), out);
		}
	}
	std::cout << "Copied " << SumSize / 1e6 << "mb of Binaries to output" << std::endl;
	//copy assets


	std::cout << "**********Cook Complete**********" << std::endl;

}
void Cooker::CreatePackage()
{

}

