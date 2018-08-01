#pragma once
#include "EngineGlobals.h"
#include <String.h>
#include <iostream>
namespace FileUtils
{
	CORE_API bool File_ExistsTest(const std::string& name, bool Silent = true);
	bool CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot);
}