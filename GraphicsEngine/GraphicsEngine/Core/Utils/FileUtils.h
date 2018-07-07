#pragma once
#include <String.h>
#include <iostream>
namespace FileUtils
{
	bool File_ExistsTest(const std::string& name, bool Silent = true);
	bool TryCreateDirectory(const std::string & name);
	bool CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot);
}