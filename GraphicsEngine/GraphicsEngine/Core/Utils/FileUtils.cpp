#include "stdafx.h"
#include "FileUtils.h"

bool FileUtils::exists_test3(const std::string & name)
{
	struct stat buffer;
	if ((stat(name.c_str(), &buffer) == 0))
	{
		return true;
	}
	std::cout << "File Does not exist " << name.c_str() << std::endl;
	return false;
}
#include <Windows.h>
#include "StringUtil.h"
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
#include <vector>
std::vector<std::string> Split(std::string Target, char c)
{
	std::vector<std::string> output;
	while (Target.find(c) != -1)
	{
		int index = Target.find(c);
		std::string NewEntry = Target;
		NewEntry.erase(NewEntry.begin() + index, NewEntry.end());
		if (NewEntry.length() == 0)
		{
			std::string tmp;
			tmp += c;
			StringUtils::RemoveChar(Target, tmp);
		}
		else
		{
			StringUtils::RemoveChar(Target, NewEntry);
		}
		if (NewEntry.length() != 0)
		{
			output.push_back(NewEntry);
		}
	}
	if (Target.length() != 0)
	{
		output.push_back(Target);
	}
	return output;
}
bool FileUtils::CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot)
{
	if (!RelativeToRoot)
	{
		StringUtils::RemoveChar(Path, root);
	}
	if (FileUtils::exists_test3(root + Path))
	{
		return true;
	}
	std::vector<std::string> split = Split(Path, '\\');
	std::string FirstPath = root;
	for (int i = 0; i < split.size(); i++)
	{
		FirstPath += "\\" + split[i];
		if (!FileUtils::exists_test3(FirstPath))
		{
			if (!FileUtils::TryCreateDirectory(FirstPath))
			{
				return false;
			}
		}
	}
	return false;
}