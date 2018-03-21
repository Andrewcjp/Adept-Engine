#include "Engine.h"
#include "GameWindow.h"
#include "D3D11/D3D11Window.h"
#include "Resource.h"
#include "EngineGlobals.h"
#include "Physics/PhysicsEngine.h"
#include "RHI/RHI.h"
#include "Editor/EditorWindow.h"
#include "Components\CompoenentRegistry.h"
#include "BaseWindow.h"
#include "Core/Assets/AssetManager.h"
#include "Game.h"
#include "Shlwapi.h"
#include "../D3D12/D3D12Window.h"
#include "../Packaging/Cooker.h"
#include "../Core/Utils/FileUtils.h"
#include "../Core/Utils/WindowsHelper.h"
#pragma comment(lib, "shlwapi.lib")
float Engine::StartTime = 0;
Game* Engine::mgame = nullptr;
CompoenentRegistry* Engine::CompRegistry = nullptr;
PhysicsEngine* Engine::PhysEngine = NULL;
#ifdef BUILD_GAME
#define UseDevelopmentWindows 0
#else
#define UseDevelopmentWindows 1
#endif

std::string Engine::GetRootDir()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
#if !BUILD_PACKAGE 
	PathCombine(buffer, buffer, L"..");
#endif
	std::wstring ws(buffer);
	return std::string(ws.begin(), ws.end());
}

Engine::Engine()
{
	StartTime = (float)PerfManager::get_nanos();
	std::cout << "Starting In " << GetRootDir() << std::endl;
	std::cout << "Loading Engine v0.1" << std::endl;
#if BUILD_PACKAGE
	std::string assetpath = GetRootDir();
	assetpath.append("\\asset\\");
	if (!FileUtils::exists_test3(assetpath))
	{
		WindowsHelpers::DisplayMessageBox("Error", " Asset Folder Not Found ");
		exit(-1);
	}
#endif

#if PHYSX_ENABLED
	PhysEngine = new PhysicsEngine();
#else
	PhysEngine = new PhysicsEngine();
	std::cout << "WARNING: Physx Disabled" << std::endl;
#endif
	if (PhysEngine != nullptr)
	{
		PhysEngine->initPhysics();
	}
	AssetManager::StartAssetManager();
	CompRegistry = new CompoenentRegistry();

}


Engine::~Engine()
{

}
void Engine::Destory()
{
	PhysEngine->cleanupPhysics();
}

void Engine::SetHInstWindow(HINSTANCE inst)
{
	m_hInst = inst;
}

RenderWindow * Engine::GetWindow()
{
	return m_appwnd;
}

ERenderSystemType Engine::GetCurrentSystem()
{
	return CurrentRenderSystem;
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
		if (input.compare("-cook") == 0)
		{
			std::cout << "Starting Cook" << std::endl;
			ShouldRunCook = true;
		}
		if (input.compare("-ForceAPI OGL") == 0)
		{
			ForcedRenderSystem = RenderSystemOGL;
			std::cout << "Forcing RenderSystemOGL" << std::endl;
		}
		else if (input.compare("-ForceAPI D3D12") == 0)
		{
			ForcedRenderSystem = RenderSystemD3D12;
			std::cout << "Forcing RenderSystemD3D12" << std::endl;
		}
	}
	if (ShouldRunCook)
	{
		RunCook();
		exit(0);//todo: proper exit
	}
	if (FullScreen)
	{
		CreateApplicationWindow(1920, 1080, ERenderSystemType::RenderSystemOGL);
	}
	else
	{
		CreateApplicationWindow(1280, 720, ERenderSystemType::RenderSystemOGL);
	}
}
void Engine::RunCook()
{
	Cooker* cook = new Cooker(AssetManager::instance);
	cook->CopyToOutput();
	delete cook;
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

#if UseDevelopmentWindows
		if (type == RenderSystemOGL)
		{
#endif
			if (ForcedRenderSystem == ERenderSystemType::Limit)
			{
#if 0
				RHI::InitRHI(RenderSystemOGL);
#else 
				RHI::InitRHI(RenderSystemD3D12);
#endif
			}
			else
			{
				RHI::InitRHI(ForcedRenderSystem);
			}
#if WITH_EDITOR
			m_appwnd = new EditorWindow(Deferredmode);
#else 
			m_appwnd = new GameWindow(/*Deferredmode*/);
#endif 
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);

#if UseDevelopmentWindows
		}
		else if (type == RenderSystemD3D12)
		{
			RHI::InitRHI(RenderSystemD3D12);
			m_appwnd = new D3D12Window();
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);
		}
		else if (type == RenderSystemD3D11)
		{

#if BUILD_D3D11
			RHI::InitRHI(RenderSystemD3D11);
			m_appwnd = new D3D11Window();
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);
#else
			isWindowVaild = false;
#endif
		}
#endif
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
