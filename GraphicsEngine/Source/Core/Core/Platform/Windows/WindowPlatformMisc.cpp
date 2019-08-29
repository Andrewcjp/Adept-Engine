#include "WindowPlatformMisc.h"
#include "WindowPlatformMisc.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/MinWindows.h"
#include <combaseapi.h>
#include <Psapi.h>
#include "dbghelp.h"

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
		Info.WorkingSetSize = (uint64)Data.WorkingSetSize;
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
	int value = (int)SetThreadAffinityMask(GetCurrentThread(), MAsk);
	ensure(value != 0);
}

std::string WindowPlatformMisc::DebugPrintLineFromAddress(void * pAddress)
{
	std::string Data;
	static int s_iState = -1;    // -1 = uninitialised, 0 = initialised ok, 1 = failed to initialise

	if (s_iState)
	{
		if (s_iState < 0)
		{
			if (SymInitialize(GetCurrentProcess(), nullptr, TRUE))
			{
				s_iState = 0;
			}
			else
			{    // Failed
				s_iState = 1;
			}
		}

		if (s_iState)
		{
			return std::to_string((uint64_t)pAddress);
		}
	}

	IMAGEHLP_LINEW64 ihl;
	ihl.SizeOfStruct = sizeof ihl;

	DWORD dwDisp;

	if (SymGetLineFromAddrW64(GetCurrentProcess(), DWORD64(pAddress), &dwDisp, &ihl) == TRUE)
	{
		//SlDebugPrintApp(SlInfo, "%ls(%d) : 0x%p", ihl.FileName, ihl.LineNumber, pAddress);
		char* buffer = new char[256];
		sprintf_s(buffer, 256, "%ls(%d) : 0x%p", ihl.FileName, ihl.LineNumber, pAddress);
		Data = buffer;
		delete[] buffer;
	}
	else
	{
		char* buffer = new char[256];
		sprintf_s(buffer, 256, "0x%p", pAddress);
		Data = buffer;
		delete[] buffer;
	}
	return Data;
}

StackTrace WindowPlatformMisc::CaptureStack(int StackOffset /*= 0*/)
{
	StackTrace Trace = StackTrace();
	RtlCaptureStackBackTrace(StackOffset + 1, 255, Trace.Stack, 0);
	return Trace;
}
