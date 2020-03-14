#pragma once
#include "Generic/GenericPlatformMisc.h"
#include "Generic/GenericWindow.h"
#include "Generic/GenericApplication.h"
#ifdef PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#include "Windows/WindowPlatformMisc.h"
#include "Windows/WindowsWindow.h"
#elif defined(PLATFORM_LINUX)
#include "Linux/LinuxApplication.h"
#include "Linux/LinuxPlatformMisc.h"
#include "Linux/LinuxWindow.h"
#elif defined(PLATFORM_ANDROID)
#include "Android/AndroidApplication.h"
#include "Android/AndroidPlatformMisc.h"
#include "Android/AndroidWindow.h"
#else
#if CHECK_INCLUDE( "Extra/ExtraPlatformCore.h")
#include "Extra/ExtraPlatformCore.h"
#endif
#endif