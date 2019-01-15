
#include "WindowPlatformMisc.h"
#include "Core\Platform\Windows\WindowPlatformMisc.h"
#include "Core/MinWindows.h"

void WindowPlatformMisc::LogPlatformOutput(FString data)
{
	OutputDebugString(data.ToWideString().c_str());
}

void WindowPlatformMisc::SetConsoleOutputColour(int colour)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, colour);
}
