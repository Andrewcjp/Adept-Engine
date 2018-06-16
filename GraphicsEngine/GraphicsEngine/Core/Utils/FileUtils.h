#pragma once
#include <String.h>
#include <iostream>
namespace FileUtils
{
	bool exists_test3(const std::string& name, bool Silent = true);
	bool TryCreateDirectory(const std::string & name);
	bool CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot);
}