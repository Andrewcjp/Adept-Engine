#pragma once
#include "Core/Types/FString.h"
#include "Core/Platform/Generic/GenericPlatformMisc.h"
class WindowPlatformMisc :public GenericPlatformMisc
{
public:
	static void LogPlatformOutput(FString data);
	static void SetConsoleOutputColour(int colour);
};

typedef WindowPlatformMisc PlatformMisc;