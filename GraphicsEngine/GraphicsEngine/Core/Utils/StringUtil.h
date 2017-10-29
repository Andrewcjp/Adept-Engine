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
			//todo: check this still work correctly
			target.erase(targetnum, charater.length());
		}
	}

	static std::wstring ConvertStringToWide(std::string target)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(target);
	}

}