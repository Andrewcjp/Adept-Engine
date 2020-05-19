#pragma once
#include <string>
#include "Core/Platform/Logger.h"
#include "Core/Platform/PlatformTypes.h"
struct PlatformMemoryInfo
{
	uint64 WorkingSetSize = 0;
	float GetWorkingSetInMB();
};
struct StackTrace
{
	void* Stack[255];
	CORE_API void PrintStack(Log::Severity Sev = Log::Severity::Warning);
	CORE_API bool operator==(const StackTrace& other)const;
};
class CORE_API GenericPlatformMisc
{
public:
	static std::string GetDateTimeString();
	static void LogPlatformOutput(FString data);
	static void SetConsoleOutputColour(int colour);
	static size_t GenerateGUID();
	static PlatformMemoryInfo GetMemoryInfo();
	static void SetCurrnetThreadAffinity(int core);
	static std::string DebugPrintLineFromAddress(void * pAddress);
	static StackTrace CaptureStack(int StackOffset = 0);
};