#pragma once

#include <String.h>
#include <iostream>
namespace FileUtils
{
	CORE_API bool File_ExistsTest(const std::string& name, bool Silent = true);
	bool CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot);
	CORE_API bool CreateDirectoriesToFullPath(std::string Path);
}