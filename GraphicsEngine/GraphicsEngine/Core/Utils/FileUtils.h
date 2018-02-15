#pragma once
#include <String.h>
#include <iostream>
namespace FileUtils
{
	bool exists_test3(const std::string& name);
	bool TryCreateDirectory(const std::string & name);
}