#pragma once
#ifndef _WINDOWINC
#define _WINDOWINC
#define NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif
#if !defined(PLATFORM_WINDOWS)
#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP
#include <Windows.h>
#endif
#undef max
#undef min
#endif