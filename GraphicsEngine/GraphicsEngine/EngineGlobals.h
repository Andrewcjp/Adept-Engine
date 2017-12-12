#pragma once
#ifdef _DEBUG
#define PHYSX_ENABLED 1
#else
#define PHYSX_ENABLED 1
#endif

#define BUILD_WINDOW_ENGINE 1
#define NO_GEN_CONTEXT 0
#define BUILD_D3D11 1
#define BUILD_OPENGL 1
#define BUILD_D3D12 1
#define BUILD_Vulkan 0

#ifdef BUILD_GAME
#define WITH_EDITOR 0
#else
#define WITH_EDITOR 1
#endif
enum ERenderSystemType
{
	RenderSystemD3D11,
	RenderSystemOGL,
	RenderSystemD3D12,
	RenderSystemVulkan
};

template <class T>
void UNUSED_PARAM(T const&)
{}


#if BUILD_D3D12
#define NAME_D3D12_OBJECT(x) SetName(x, L#x)
typedef struct ID3D12GraphicsCommandList CommandListDef;
#else 
typedef struct Stub{} CommandListDef;
#endif
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