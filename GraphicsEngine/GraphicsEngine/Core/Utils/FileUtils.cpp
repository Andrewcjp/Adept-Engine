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
