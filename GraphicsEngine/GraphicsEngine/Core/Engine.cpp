#include "Engine.h"
//#include "OpenGL/OGLWindow.h"
#include "GameWindow.h"
#include "D3D11/D3D11Window.h"
#include "Resource.h"
#include "EngineGlobals.h"
#include "Physics/PhysicsEngine.h"
#include <Windows.h>
#if PHYSX_ENABLED
#include "Physics/PhysxEngine.h"
#endif
#include "RHI/RHI.h"
#include "Editor/EditorWindow.h"
#include "Components\CompoenentRegistry.h"
PhysicsEngine* Engine::PhysEngine = NULL;
PhysxEngine* Engine::PPhysxEngine = NULL;
#include "BaseWindow.h"
CompoenentRegistry* Engine::CompRegistry = nullptr;
#include "Core/Assets/AssetManager.h"
#include "Game.h"
#include "Shlwapi.h"
#include "../D3D12/D3D12Window.h"
#pragma comment(lib, "shlwapi.lib")
float Engine::StartTime = 0;
Game* Engine::mgame = nullptr;
EditorWindow * Engine::GetEditorWindow()
{
	return reinterpret_cast<EditorWindow*>(m_appwnd);
}

std::string Engine::GetRootDir()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	PathCombine(buffer, buffer, L"..");
	std::wstring ws(buffer);
	return std::string(ws.begin(), ws.end());
}

Engine::Engine()
{
	StartTime = (float)PerfManager::get_nanos();
	std::cout << "Starting In " << GetRootDir() << std::endl;
	std::cout << "Loading Engine v0.1" << std::endl;
#if PHYSX_ENABLED
	PPhysxEngine = new PhysxEngine();
	PhysEngine = PPhysxEngine;
#else
	PhysEngine = new PhysicsEngine();
	std::cout << "WARNING: Physx Disabled" << std::endl;
#endif
	if (PhysEngine != nullptr)
	{
		PhysEngine->initPhysics(false);
	}
	AssetManager::StartAssetManager();
	CompRegistry = new CompoenentRegistry();

}


Engine::~Engine()
{

}
void Engine::Destory()
{
	PhysEngine->cleanupPhysics(false);
}


void Engine::CreateApplication(HINSTANCE, LPSTR args, int nCmdShow)
{
	if (nCmdShow > 0)
	{
		std::string input = args;
		if (input.compare("-deferred") == 0)
		{
			std::cout << "Starting in Deferred Rendering mode" << std::endl;
			Deferredmode = true;
		}
		if (input.compare("-fullscreen") == 0)
		{
			FullScreen = true;
		}
	}
	if (FullScreen)
	{
		//CreateApplicationWindow(1920, 1080, ERenderSystemType::RenderSystemOGL);
	}
	else
	{
		CreateApplicationWindow(1280, 720,(0) ? ERenderSystemType::RenderSystemD3D12: ERenderSystemType::RenderSystemOGL);
	}


}
void Engine::SetContextData(HGLRC hglrc, HWND hwnd, HDC hdc)
{
	mhglrc = hglrc;
	mhwnd = hwnd;
	mhdc = hdc;
}

void Engine::SetGame(Game * game)
{
	mgame = game;
	CompRegistry->RegisterExtraComponents(game->GetECR());
}

Game * Engine::GetGame()
{
	return mgame;
}

void Engine::CreateApplicationWindow(int width, int height, ERenderSystemType type)
{
	if (m_appwnd == nullptr)
	{

		mwidth = width;
		mheight = height;
		CurrentRenderSystem = type;
		if (type == RenderSystemD3D11)
		{

#if BUILD_D3D11
			RHI::InitRHI(RenderSystemD3D11);
			m_appwnd = new D3D11Window();
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);
#else
			isWindowVaild = false;
#endif

		}
		else if (type == RenderSystemOGL)
		{
#if BUILD_OPENGL
#if 1
			RHI::InitRHI(RenderSystemOGL);
#else 
			RHI::InitRHI(RenderSystemD3D12);
#endif
#if WITH_EDITOR
			m_appwnd = new EditorWindow(Deferredmode);
#else 
			m_appwnd = new GameWindow(/*Deferredmode*/);
#endif 

			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);
#else 
			isWindowVaild = false;
#endif			
		}
		else if (type == RenderSystemD3D12)
		{
			RHI::InitRHI(RenderSystemD3D12);
			m_appwnd = new D3D12Window();
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);
		}
		if (!isWindowVaild)
		{
			std::cout << "Fatal Error: Window Invalid" << std::endl;
		}
#if !NO_GEN_CONTEXT
		m_appwnd->SetVisible(TRUE);
#endif

	}
}
void Engine::setVSync(bool sync)
{
	// Function pointer for the wgl extention function we need to enable/disable
	// vsync
	typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glGetString(GL_EXTENSIONS);
	if (extensions == nullptr)
	{
		return;
	}
	if (strstr(extensions, "WGL_EXT_swap_control") == 0)
	{
		std::cout << "WGL_EXT_swap_control Not Avalible" << std::endl;
		return;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

		if (wglSwapIntervalEXT != nullptr)
		{
			wglSwapIntervalEXT(sync);
		}
		else
		{
			std::cout << "wglSwapIntervalEXT Not valid" << std::endl;
		}
	}
}
bool Engine::SwitchRenderAPI(ERenderSystemType type)
{
	if (type == CurrentRenderSystem)
	{
		return false;
	}
	isWindowVaild = false;
	m_appwnd->DestroyRenderWindow();
	delete m_appwnd;
	m_appwnd = nullptr;
	RHI::InitRHI(type);
	CreateApplicationWindow(mwidth, mheight, type);
	return true;
}
