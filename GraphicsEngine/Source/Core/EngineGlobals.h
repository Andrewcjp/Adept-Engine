#pragma once
#ifdef USE_PHYSX
#define PHYSX_ENABLED 1
#define TDSIM_ENABLED 0
#else
#define PHYSX_ENABLED 0
#define TDSIM_ENABLED 0
#endif
#if PHYSX_ENABLED && TDSIM_ENABLED
#error "Two physics engines enabled only one is allowed";
#endif
#define NO_GEN_CONTEXT 0

#define BUILD_D3D11 0
#define BUILD_OPENGL 0
#define BUILD_D3D12 1
#ifdef WITH_VK
#define BUILD_VULKAN 1
#else
#define BUILD_VULKAN 0
#endif
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
#define RUNTESTS 0
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

#if !defined(PLATFORM_ANDROID) || !defined(PLATFORM_LINUX)
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT
#define DLLIMPORT
#endif
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

#if defined(CORE_EXPORT) 
#   define TEMP_API DLLEXPORT
#else 
#ifndef STATIC_MODULE
#   define TEMP_API DLLIMPORT
#else
#   define TEMP_API
#endif
#endif // TEMP_API

//Utility Marcos
#define SafeDelete(Target)if(Target != nullptr){delete Target; Target= nullptr;}
#define SafeRelease(Target) if(Target != nullptr){Target->Release(); Target= nullptr;}

//RHI Optimization
#if 0 //defined(ALLOW_SINGLE_RHI)
#undef BUILD_D3D12 
#undef BUILD_VULKAN
#if SINGLERHI_DX12
#define BUILD_D3D12 1
#define BUILD_VULKAN 0
#elif SINGLERHI_VK
#define BUILD_D3D12 0
#define BUILD_VULKAN 1
#endif
#define USE_SINGLE_RHI 0
#else
#define USE_SINGLE_RHI 0
#endif
#if USE_SINGLE_RHI 
#define RHI_VIRTUAL   
#else
#define RHI_VIRTUAL virtual
#endif
#if BUILD_SHIPPING
#define DETECT_MEMORY_LEAKS 0
#else
#define DETECT_MEMORY_LEAKS 0
#endif


#define PROPERTY()
#define UCLASS()
#define CLASS_BODY() void ProcessSerialArchive(Archive* A);
#define GENHASH() size_t  GetHash();

#define RESTRICT __restrict
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN_INNER(x, y)
#define PREPROCESSOR_JOIN_INNER(x, y) x##y

extern void ADNOP();

#if !defined(PLATFORM_ANDROID)
#define CHECK_INCLUDE(x) __has_include(x)
#else
#define CHECK_INCLUDE(x) 0
#endif


#if !defined(PLATFORM_WINDOWS)
#define BUILD_WISE 0

#define SUPPORTS_COOK 0
#else
#define BUILD_WISE 0

#define SUPPORTS_COOK 1
#endif
#if defined(PLATFORM_WINDOWS)
#ifdef SUPPORT_OPENVR
#define BUILD_STEAMVR 1
#else
#define BUILD_STEAMVR 0
#endif
#endif

#ifdef SUPPORT_FREETYPE
#define BUILD_FREETTYPE 1
#else
#define BUILD_FREETTYPE 0
#endif
#include "AdditionalPlatforms.h"
namespace EPlatforms
{
	enum Type
	{
		Windows,
		Windows_VK,
		Windows_DX12,
		Linux,
		Android,
		GEN_ADD_PLATFORMS
		Limit
	};
	std::string ToString(EPlatforms::Type type);
	EPlatforms::Type Parse(std::string name);
};
#ifdef PLATFORM_WINDOWS
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE  
#endif

#include "PlatformConfig.h"
#include "Core/Maths/Math.h"