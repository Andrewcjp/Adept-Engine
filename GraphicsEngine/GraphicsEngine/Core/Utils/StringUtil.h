#pragma once
#include <locale>
#include <codecvt>
#include <string>
namespace StringUtils
{
	static void RemoveChar(std::string &target, std::string charater)
	{
		size_t targetnum = target.find(charater);
		if (targetnum != -1)
		{
			target.erase(targetnum, charater.length());
		}
	}

	static std::wstring ConvertStringToWide(std::string target)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(target);
	}
	static std::string ConvertWideToString(std::wstring target)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(target);
	}
}