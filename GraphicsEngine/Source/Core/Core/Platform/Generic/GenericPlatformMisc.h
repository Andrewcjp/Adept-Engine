#pragma once
#include <string>
struct PlatformMemoryInfo
{
	uint WorkingSetSize = 0;
	float GetWorkingSetInMB();
};
class CORE_API GenericPlatformMisc
{
public:
	static std::string GetDateTimeString();
};