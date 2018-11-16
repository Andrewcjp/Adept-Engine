#pragma once
#include <locale>
#include <codecvt>
#include <string>
#include <algorithm>

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
	static void ToLower(std::string& Target)
	{
		std::transform(Target.begin(), Target.end(), Target.begin(), ::tolower);
	}
	static std::vector<std::string> Split(std::string Target, char c)
	{
		std::vector<std::string> output;
		while (Target.find(c) != -1)
		{
			size_t index = Target.find(c);
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
	static const char* CopyStringToCharArray(std::string String)
	{
		char* tmp = new char[String.length() + 1];
		strcpy_s(tmp, String.length() + 1, String.c_str());
		return tmp;
	}

}