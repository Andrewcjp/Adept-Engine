#pragma once
#define PHYSX_ENABLED 1
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
#else
#define STATS 1
#define DOCHECK 1
#define DOFULLCHECK 1
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
/*
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "SOIL.lib")
#pragma comment(lib, "assimp.lib")
#pragma comment(lib, "PhysX3CHECKED_x64.lib")
#pragma comment(lib, "PhysX3CommonCHECKED_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsCHECKED.lib")
#pragma comment(lib, "PxPvdSDKCHECKED_x64.lib")
#pragma comment(lib, "PhysX3CookingCHECKED_x64.lib")
#pragma comment(lib, "PhysX3CharacterKinematicCHECKED_x64.lib")
#pragma comment(lib, "PxTaskCHECKED_x64.lib")
#pragma comment(lib, "SceneQueryCHECKED.lib")
#pragma comment(lib, "SimulationControllerCHECKED.lib")
#pragma comment(lib, "PxFoundationCHECKED_x64.lib")

#pragma comment(lib, "TestGame.lib")
*/