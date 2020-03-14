#pragma once

#if CHECK_INCLUDE("Core/Platform/Extra/ExtraPlatformCore.h")
#define PLATFORM_CONFIG
#include "Core/Platform/Extra/ExtraPlatformCore.h"
#undef PLATFORM_CONFIG
#endif
#ifndef OVERRIDE_PLATFORM_CONFIG
#include "Core/Platform/PlatformInterface.h"
typedef  BasePlatformInterface PlatformInterface;
#endif