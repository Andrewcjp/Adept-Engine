#pragma once
#ifdef USE_PHYSX
#define PHYSX_ENABLED 0
#define TDSIM_ENABLED 1
#else
#define PHYSX_ENABLED 0
#define TDSIM_ENABLED 1
#endif
#if PHYSX_ENABLED && TDSIM_ENABLED
#error "Two physics engines enabled only one is allowed";
#endif
#define NO_GEN_CONTEXT 0

#define BUILD_D3D11 0
#define BUILD_OPENGL 0
#define BUILD_D3D12 1
#define BUILD_VULKAN 0
#ifdef BUILD_SHIP
#define BUILD_SHIPPING 1
#else
#define BUILD_SHIPPING 0
#endif
#if BUILD_SHIPPING
#define STATS 0
#define DOCHECK 1
#define DOFULLCHECK 0
#define RUNTESTS 0
#else
#define STATS 1
#define DOCHECK 1
#define DOFULLCHECK 1
#define RUNTESTS 1
#endif

#if STATS
#define GPUTIMERS_FULL 1
#else
#define GPUTIMERS_FULL 0
#endif 

#ifdef BUILD_GAME
#define WITH_EDITOR 0
#define BUILD_PACKAGE 1
#else
#define WITH_EDITOR 1
#define BUILD_PACKAGE 0
#endif
enum ERenderSystemType
{
	RenderSystemD3D11,
	RenderSystemOGL,
	RenderSystemD3D12,
	RenderSystemVulkan,
	Limit
};

template <class T>
void UNUSED_PARAM(T const&)
{}
#include "Core\Asserts.h"

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#if defined(CORE_EXPORT) 
#   define RHI_API DLLEXPORT
#else 
#ifndef STATIC_MODULE
#   define RHI_API DLLIMPORT
#else 
#   define RHI_API
#endif
#endif // RHI_API

#if defined(CORE_EXPORT) 
#   define CORE_API DLLEXPORT
#else 
#ifndef STATIC_MODULE
#   define CORE_API DLLIMPORT
#else
#   define CORE_API
#endif
#endif // CORE_API

//Asserts
#define PLATFORM_WINDOWS 1

//Utility Marcos
#define SafeDelete(Target)if(Target != nullptr){delete Target; Target= nullptr;}
#define SafeRelease(Target) if(Target != nullptr){Target->Release(); Target= nullptr;}
