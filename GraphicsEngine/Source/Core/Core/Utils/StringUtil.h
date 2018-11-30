#pragma once
#include <string>
namespace StringUtils
{
	CORE_API void RemoveChar(std::string &target, std::string charater);
	CORE_API std::wstring ConvertStringToWide(std::string target);
	CORE_API std::string ConvertWideToString(std::wstring target);
	CORE_API void ToLower(std::string& Target);
	CORE_API std::vector<std::string> Split(std::string Target, char c);
	CORE_API const char* CopyStringToCharArray(std::string String);
}