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
	if (CreateDirectory(StringUtils::ConvertStringToWide(name).c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		return true;
	}
	return false;
}