#include "Engine.h"
#include "GameWindow.h"
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
#include "Packaging/Cooker.h"
#include "Core/Utils/FileUtils.h"
#include "Module/ModuleManager.h"


#pragma comment(lib, "shlwapi.lib")

float Engine::StartTime = 0;
Game* Engine::mgame = nullptr;
CORE_API CompoenentRegistry* Engine::CompRegistry = nullptr;
PhysicsEngine* Engine::PhysEngine = nullptr;
Engine* Engine::EngineInstance = nullptr;


PhysicsEngine * Engine::GetPhysEngineInstance()
{
	return PhysEngine;
}

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
	EngineInstance = this;
	StartTime = (float)PerfManager::get_nanos();
	Log::OutS << "Starting In " << GetRootDir() << Log::OutS;
	Log::OutS << "Loading Engine v0.1" << Log::OutS;
#if BUILD_PACKAGE
	std::string assetpath = GetRootDir();
	assetpath.append("\\asset\\");
	if (!FileUtils::File_ExistsTest(assetpath))
	{
		PlatformApplication::DisplayMessageBox("Error", " Asset Folder Not Found ");
		exit(-1);
	}
#endif
#if PHYSX_ENABLED
	PhysEngine = new PhysicsEngine();
#else
	PhysEngine = new PhysicsEngine();
	Log::OutS << "WARNING: Physx Disabled" << Log::OutS;
#endif
	if (PhysEngine != nullptr)
	{
		PhysEngine->initPhysics();
	}
	AssetManager::StartAssetManager();
	CompRegistry = new CompoenentRegistry();
	FString::RunFStringTests();
	ModuleManager::Get()->PreLoadModules();
}

Engine::~Engine()
{

}
void Engine::PreInit()
{
	PerfManager::StartPerfManager();

}
void Engine::OnRender()
{
	if (m_appwnd != nullptr)
	{
		m_appwnd->Render();
	}
}

void Engine::OnDestoryWindow()
{

}

void Engine::Destory()
{
	PhysEngine->cleanupPhysics();
	ModuleManager::Get()->ShutDown();
	PerfManager::ShutdownPerfManager();
}

void Engine::LoadGame()
{
	GameModule* Gamemodule = ModuleManager::Get()->GetModule<GameModule>("TestGame");
	//ensure(Gamemodule);
	if (Gamemodule == nullptr)
	{
		return;
	}
	Game* gm = Gamemodule->GetGamePtr(CompRegistry);
	//	ensure(gm);
	SetGame(gm);
}

RenderWindow * Engine::GetRenderWindow()
{
	return m_appwnd;
}

void Engine::CreateApplication()
{
	CreateApplicationWindow(GetWidth(), GetHeight());
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
	game->SetTickRate(20);
}

Game * Engine::GetGame()
{
	return mgame;
}

void Engine::ProcessCommandLineInput(FString args, int nCmdShow)
{
	mwidth = 1280;
	mheight = 720;
	if (nCmdShow > 0)
	{
		//todo: xbyx
		std::string input = args.ToSString();
		if (input.compare("-deferred") == 0)
		{
			Log::OutS << "Starting in Deferred Rendering mode" << Log::OutS;
			Deferredmode = true;
		}
		if (input.compare("-fullscreen") == 0)
		{
			FullScreen = true;
		}
		if (input.compare("-cook") == 0)
		{
			Log::OutS << "Starting Cook" << Log::OutS;
			ShouldRunCook = true;
		}
		else if (input.compare("-dx12") == 0)
		{
			ForcedRenderSystem = RenderSystemD3D12;
			Log::OutS << "Forcing RenderSystem D3D12" << Log::OutS;
		}
		else if (input.compare("-vk") == 0)
		{
			ForcedRenderSystem = RenderSystemVulkan;
			Log::OutS << "Forcing RenderSystem Vulkan" << Log::OutS;
		}
	}

	if (ShouldRunCook)
	{
		RunCook();
		exit(0);//todo: proper exit
	}

}

int Engine::GetWidth()
{
	return mwidth;
}

int Engine::GetHeight()
{
	return mheight;
}

void Engine::Resize(int width, int height)
{
	mwidth = width;
	mheight = height;
}

IntPoint * Engine::GetInitalScreen()
{
	return &IntialScreenSize;
}

void Engine::CreateApplicationWindow(int width, int height)
{
	if (m_appwnd == nullptr)
	{
		mwidth = width;
		mheight = height;
#if UseDevelopmentWindows
		if (type == RenderSystemD3D12)
		{
			RHI::InitRHI(RenderSystemD3D12);
			m_appwnd = new D3D12Window();
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height, FullScreen);
			m_appwnd->SetVisible(TRUE);
			return;
		}
#endif
		if (ForcedRenderSystem == ERenderSystemType::Limit)
		{
			RHI::InitRHI(RenderSystemD3D12);
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
		isWindowVaild = m_appwnd->CreateRenderWindow(width, height);

		if (!isWindowVaild)
		{
			Log::OutS << "Fatal Error: Window Invalid" << Log::OutS;
		}
	}
}

