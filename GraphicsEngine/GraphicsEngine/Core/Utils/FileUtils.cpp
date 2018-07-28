#include "stdafx.h"
#include "FileUtils.h"
#include <vector>
#include "Core/Platform/Logger.h"
#include "StringUtil.h"
#include "Core/Platform/PlatformCore.h"
bool FileUtils::File_ExistsTest(const std::string & name,bool Silent)
{
	struct stat buffer;
	if ((stat(name.c_str(), &buffer) == 0))
	{
		return true;
	}
	if (!Silent)
	{
		Log::OutS  << "File Does not exist " << name.c_str() << Log::OutS;
	}
	return false;
}
 
bool FileUtils::CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot)
{
	if (!RelativeToRoot)
	{
		StringUtils::RemoveChar(Path, root);
	}
	if (FileUtils::File_ExistsTest(root + Path))
	{
		return true;
	}
	std::vector<std::string> split = StringUtils::Split(Path, '\\');
	std::string FirstPath = root;
	for (int i = 0; i < split.size(); i++)
	{
		FirstPath += "\\" + split[i];
		if (!FileUtils::File_ExistsTest(FirstPath))
		{
			if (!PlatformApplication::TryCreateDirectory(FirstPath))
			{
				return false;
			}
		}
	}
	return false;
}