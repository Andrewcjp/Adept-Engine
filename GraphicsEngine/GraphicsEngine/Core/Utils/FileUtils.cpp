#include "stdafx.h"
#include "FileUtils.h"
#include <vector>

#include "StringUtil.h"
bool FileUtils::File_ExistsTest(const std::string & name,bool Silent)
{
	struct stat buffer;
	if ((stat(name.c_str(), &buffer) == 0))
	{
		return true;
	}
	if (!Silent)
	{
		std::cout << "File Does not exist " << name.c_str() << std::endl;
	}
	return false;
}


bool FileUtils::TryCreateDirectory(const std::string & name)
{
	DWORD LastError;
	if (CreateDirectory(StringUtils::ConvertStringToWide(name).c_str(), NULL))
	{
		return true;
	}
	else
	{
		LastError = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(LastError);
		if (LastError == ERROR_ALREADY_EXISTS)
		{
			return true;
		}
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
			if (!FileUtils::TryCreateDirectory(FirstPath))
			{
				return false;
			}
		}
	}
	return false;
}