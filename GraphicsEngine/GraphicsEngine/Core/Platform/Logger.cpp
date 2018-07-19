#include "Stdafx.h"
#include "Logger.h"
#include "../Core/Utils/StringUtil.h"
#include "../Core/Platform/PlatformCore.h"
Log::StreamWrapper Log::OutS;


void Log::LogOutput(std::string data)
{
	PlatformMisc::LogPlatformOutput(data);
	printf(data.c_str());
}
