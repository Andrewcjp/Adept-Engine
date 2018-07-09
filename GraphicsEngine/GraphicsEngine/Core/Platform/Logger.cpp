#include "Stdafx.h"
#include "Logger.h"
#include "../Core/Utils/StringUtil.h"
Log::StreamWrapper Log::OutS;


void Log::LogOutput(std::string data)
{
	OutputDebugString(StringUtils::ConvertStringToWide(data).c_str());
	printf(data.c_str());
}
