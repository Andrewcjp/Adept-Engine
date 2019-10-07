
#include "GenericPlatformMisc.h"
#include <ctime> 

std::string GenericPlatformMisc::GetDateTimeString()
{
	time_t t = std::time(0);   // get time now
	tm* now = new tm();
	localtime_s(now, &t);
	std::stringstream out;
	out << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << "-" << (now->tm_hour) << "-" << (now->tm_min) << "-" << (now->tm_sec);
	delete now;
	return out.str();
}

void GenericPlatformMisc::LogPlatformOutput(FString data)
{}

void GenericPlatformMisc::SetConsoleOutputColour(int colour)
{}

size_t GenericPlatformMisc::GenerateGUID()
{
	return size_t();
}

PlatformMemoryInfo GenericPlatformMisc::GetMemoryInfo()
{
	return PlatformMemoryInfo();
}

void GenericPlatformMisc::SetCurrnetThreadAffinity(int core)
{}

std::string GenericPlatformMisc::DebugPrintLineFromAddress(void * pAddress)
{
	return std::string();
}

 StackTrace GenericPlatformMisc::CaptureStack(int StackOffset)
{
	return StackTrace();
}

float PlatformMemoryInfo::GetWorkingSetInMB()
{
	return (float)WorkingSetSize / 1024.0f / 1024.0f;
}

void StackTrace::PrintStack(Log::Severity Sev /*= Log::Severity::Warning*/)
{
	Log::LogMessage("---Stack trace---", Sev);
	for (int i = 0; i < 255; i++)
	{
		if (Stack[i] == nullptr)
		{
			return;
		}
		Log::LogMessage(PlatformMisc::DebugPrintLineFromAddress(Stack[i]), Sev);
	}
}

bool StackTrace::operator==(const StackTrace & other) const
{
	for (int i = 0; i < 255; i++)
	{
		if (Stack[i] != other.Stack[i])
		{
			return false;
		}
	}
	return true;
}
