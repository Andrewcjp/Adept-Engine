#pragma once
#include "Core/Types/FString.h"
#include "Core/Platform/Generic/GenericPlatformMisc.h"
class WindowPlatformMisc :public GenericPlatformMisc
{
public:
	static void LogPlatformOutput(FString data);
	static void SetConsoleOutputColour(int colour);
	static size_t GenerateGUID();
	static PlatformMemoryInfo GetMemoryInfo();
};

typedef WindowPlatformMisc PlatformMisc;