#pragma once
#ifdef PLATFORM_WINDOWS
#include "Core/Types/FString.h"
#include "Core/Platform/Generic/GenericPlatformMisc.h"
class WindowPlatformMisc :public GenericPlatformMisc
{
public:
	static void LogPlatformOutput(FString data);
	static void SetConsoleOutputColour(int colour);
	static size_t GenerateGUID();
	static PlatformMemoryInfo GetMemoryInfo();
	static void SetCurrnetThreadAffinity(int core);
	static std::string DebugPrintLineFromAddress(void * pAddress);
	CORE_API static StackTrace CaptureStack(int StackOffset = 0);

};

typedef WindowPlatformMisc PlatformMisc;
#endif