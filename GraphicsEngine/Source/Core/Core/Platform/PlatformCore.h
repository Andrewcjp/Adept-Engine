#pragma once
#include "Generic/GenericPlatformMisc.h"
#include "Generic/GenericWindow.h"
#include "Generic/GenericApplication.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"

#include "Windows/WindowPlatformMisc.h"
//#include "Windows/WindowsWindow.h"
class WindowsWindow;
typedef WindowsWindow PlatformWindow;
#endif