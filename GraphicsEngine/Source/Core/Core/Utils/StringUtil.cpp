#include "StringUtil.h"
#include <locale>
#include <codecvt>
#include <algorithm>
#include <iomanip>
void StringUtils::RemoveChar(std::string &target, std::string charater)
{
	size_t targetnum = target.find(charater);
	if (targetnum != -1)
	{
		target.erase(targetnum, charater.length());
	}
}

std::wstring StringUtils::ConvertStringToWide(std::string target)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(target);
}

std::string StringUtils::ConvertWideToString(std::wstring target)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(target);
}

void StringUtils::ToLower(std::string & Target)
{
	std::transform(Target.begin(), Target.end(), Target.begin(), ::tolower);
}

std::vector<std::string> StringUtils::Split(std::string Target, char c)
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

const char* StringUtils::CopyStringToCharArray(std::string String)
{
	char* tmp = new char[String.length() + 1];
	strcpy_s(tmp, String.length() + 1, String.c_str());
	return tmp;
}

std::string StringUtils::ToStringFloat(float value, int Places /*= 2*/)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(Places) << value;
	return stream.str();
}
