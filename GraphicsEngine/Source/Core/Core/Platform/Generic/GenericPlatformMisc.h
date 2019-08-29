#pragma once
#include <string>
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
};