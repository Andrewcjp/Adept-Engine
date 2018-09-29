#pragma once
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#if defined(TDPHYSICS_EXPORT) 
#   define TD_API DLLEXPORT
#else 
#ifndef STATIC_MODULE
#   define TD_API DLLIMPORT
#else 
#   define TD_API
#endif
#endif // TD_API
#include <vector>