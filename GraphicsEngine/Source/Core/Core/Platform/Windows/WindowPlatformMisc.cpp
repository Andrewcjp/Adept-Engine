
#include "WindowPlatformMisc.h"
#include "Core\Platform\Windows\WindowPlatformMisc.h"
#include "Core/MinWindows.h"

void WindowPlatformMisc::LogPlatformOutput(FString data)
{
	OutputDebugString(data.ToWideString().c_str());
}
