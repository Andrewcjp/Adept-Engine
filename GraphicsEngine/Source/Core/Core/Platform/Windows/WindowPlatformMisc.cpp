#include "WindowPlatformMisc.h"
#include "WindowPlatformMisc.h"
#include "Core/MinWindows.h"
#include "Core/Platform/PlatformCore.h"
#include <combaseapi.h>
#include <Psapi.h>

void WindowPlatformMisc::LogPlatformOutput(FString data)
{
	OutputDebugString(data.ToWideString().c_str());
}

void WindowPlatformMisc::SetConsoleOutputColour(int colour)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, colour);
}

size_t WindowPlatformMisc::GenerateGUID()
{
	GUID gidReference;
	HRESULT hCreateGuid = CoCreateGuid(&gidReference);
	ensure(hCreateGuid == S_OK);
	return gidReference.Data1;
}

PlatformMemoryInfo WindowPlatformMisc::GetMemoryInfo()
{
	PlatformMemoryInfo Info = {};
	PROCESS_MEMORY_COUNTERS Data = {};
	if (GetProcessMemoryInfo(GetCurrentProcess(), &Data, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		Info.WorkingSetSize = Data.WorkingSetSize;
	}
	else
	{
		Log::LogMessage("GetProcessMemoryInfo failed with code " + std::to_string(GetLastError()), Log::Error);
	}
	return Info;
}

void WindowPlatformMisc::SetCurrnetThreadAffinity(int core)
{
	int MAsk = 1 << core;
	int value = SetThreadAffinityMask(GetCurrentThread(), MAsk);
	ensure(value != 0);
}
