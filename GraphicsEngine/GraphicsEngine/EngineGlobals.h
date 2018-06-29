#pragma once
#define PHYSX_ENABLED 1
#define BUILD_WINDOW_ENGINE 1
#define NO_GEN_CONTEXT 0

#define BUILD_D3D11 0
#define BUILD_OPENGL 0
#define BUILD_D3D12 1
#define BUILD_VULKAN 0

#define STATS 1
#define DOCHECK 1
#define BUILD_SHIPPING 0
#if STATS
#define GPUTIMERS_FULL 1
#else
#define GPUTIMERS_FULL 0
#endif

//#pragma warning (disable:4100 4505)
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

#define CORE_API __declspec(dllexport)
//Asserts

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