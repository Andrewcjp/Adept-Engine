#pragma once
#include "Core/Types/FString.h"
class WindowPlatformMisc
{
public:
	static void LogPlatformOutput(FString data);
};

typedef WindowPlatformMisc PlatformMisc;