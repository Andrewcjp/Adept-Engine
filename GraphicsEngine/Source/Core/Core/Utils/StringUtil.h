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
	CORE_API std::string ToString(double value, int Places = 2);
	CORE_API std::string ToString(float value, int Places = 2);
	//CORE_API std::string ByteToMB(uint64_t value);
	CORE_API std::string ByteToGB(uint64_t value);
	CORE_API std::string BoolToString(bool value);
	CORE_API std::string BoolToStringE(bool value);
	CORE_API bool Contains(const std::string Data, const std::string& value);
	CORE_API bool Contains(const std::wstring Data, const std::wstring & value);
	CORE_API std::string ToString(bool value);
	template<class T>
	std::string ByteToMB(T value)
	{
		return StringUtils::ToString((float)(value) / 1024 / 1024) + "MB";
	}
	CORE_API bool replace(std::string& str, const std::string& from, const std::string& to);
	CORE_API std::string PadToLength(std::string in, int length);
	CORE_API std::string GetFilename(const char * name);
}