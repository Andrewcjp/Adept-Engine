#pragma once
#include "Generic/GenericPlatformMisc.h"
#include "Generic/GenericWindow.h"
#include "Generic/GenericApplication.h"
#ifdef PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#include "Windows/WindowPlatformMisc.h"
class WindowsWindow;
typedef WindowsWindow PlatformWindow;
#elif defined(PLATFROM_LINUX)
#include "Linux/LinuxApplication.h"
#include "Linux/LinuxPlatformMisc.h"
#include "Linux/LinuxWindow.h"
#elif defined(PLATFROM_ANDROID)
#include "Android/AndroidApplication.h"
#include "Android/AndroidPlatformMisc.h"
#include "Android/AndroidWindow.h"
#endif