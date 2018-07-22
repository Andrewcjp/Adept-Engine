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

void Log::LogMessage( std::string msg, Severity s)
{
	std::string data = "";
	switch(s)
	{
	case Severity::Message:
		data.append("Message:");
		break;
	case Severity::Warning:
		data.append("Warning:");
		break;
	case Severity::Error:
		data.append("Error:");
		break;
	}
	data.append(msg);
	data.append("\n");

	PlatformMisc::LogPlatformOutput(data);
	printf(data.c_str());
}
