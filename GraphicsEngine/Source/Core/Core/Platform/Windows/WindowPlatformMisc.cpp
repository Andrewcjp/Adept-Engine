#include "WindowPlatformMisc.h"
#include "WindowPlatformMisc.h"
#include "Core/MinWindows.h"
#include "Core/Platform/PlatformCore.h"
#include <combaseapi.h>

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
