#include "Engine.h"
#include "GameWindow.h"
#include "Resource.h"

#include "Physics/PhysicsEngine.h"
#include "RHI/RHI.h"
#include "Editor/EditorWindow.h"
#include "Components\CompoenentRegistry.h"
#include "BaseWindow.h"
#include "Core/Assets/AssetManager.h"
#include "Game/Game.h"
#include "Shlwapi.h"
#include "Packaging/Cooker.h"
#include "Core/Utils/FileUtils.h"
#include "Module/ModuleManager.h"
#include "UI/UIManager.h"
#include "Input.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Performance/PerfManager.h"
#include "UI/Core/UIWidget.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Version.h"
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

std::string Engine::GetExecutionDir()
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
	Log::OutS << "Starting In " << GetExecutionDir() << Log::OutS;
	Log::OutS << "Loading Engine v" << ENGINE_VERSION << Log::OutS;
#if PHYSX_ENABLED
	Log::OutS << "Running with Physx" << Log::OutS;
#else	
	Log::OutS << "Running with TDSim" << Log::OutS;
#endif
	PhysEngine = new PhysicsEngine();
	if (PhysEngine != nullptr)
	{
		PhysEngine->initPhysics();
	}
	AssetManager::StartAssetManager();
	CompRegistry = new CompoenentRegistry();
#if RUNTESTS
	FString::RunFStringTests();
#endif
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
	m_appwnd->DestroyRenderWindow();
	m_appwnd = nullptr;
}

void Engine::Destory()
{
	RHI::DestoryContext();
	if (PhysEngine != nullptr)
	{
		PhysEngine->cleanupPhysics();
	}
	ModuleManager::Get()->ShutDown();
	PerfManager::ShutdownPerfManager();
}

void Engine::LoadGame()
{
	GameModule* Gamemodule = ModuleManager::Get()->GetModule<GameModule>("TestGame");
	ensure(Gamemodule);
	if (Gamemodule == nullptr)
	{
		return;
	}
	Game* gm = Gamemodule->GetGamePtr(CompRegistry);
	ensure(gm);
	SetGame(gm);
}

RenderWindow * Engine::GetRenderWindow()
{
	return m_appwnd;
}

void Engine::CreateApplication()
{
	if (ForcedRenderSystem == ERenderSystemType::Limit)
	{
		RHI::InitRHI(RenderSystemD3D12);
	}
	else
	{
		RHI::InitRHI(ForcedRenderSystem);
	}
	RHI::InitialiseContext();
	if (!IsCooking)
	{
		CreateApplicationWindow(GetWidth(), GetHeight());
	}
	else
	{
		RunCook();
	}

}

void Engine::RunCook()
{
	Cooker* cook = new Cooker();
	cook->CopyToOutput();
	delete cook;
}

void Engine::SetGame(Game * game)
{
	mgame = game;
	CompRegistry->RegisterExtraComponents(game->GetECR());
	game->Init();
}

Game * Engine::GetGame()
{
	return mgame;
}

static ConsoleVariable RunCookVar("cook", 0, ECVarType::LaunchOnly);
void Engine::ProcessCommandLineInput(FString args, int nCmdShow)
{
	mwidth = 1700;
	mheight = 720;
	if (nCmdShow > 0)
	{
		//todo: xbyx
		std::string input = args.ToSString();
		ConsoleVariableManager::SetupVars(input);
		if (input.compare("-fullscreen") == 0)
		{
			FullScreen = true;
		}

		if (RunCookVar.GetBoolValue())
		{
			Log::OutS << "Starting Cook" << Log::OutS;
			ShouldRunCook = true;
		}

		if (input.compare("-dx12") == 0)
		{
			ForcedRenderSystem = RenderSystemD3D12;
			Log::OutS << "Forcing RenderSystem D3D12" << Log::OutS;
		}
#if BUILD_VULKAN
		if (input.compare("-vk") == 0)
		{
			ForcedRenderSystem = RenderSystemVulkan;
			Log::OutS << "Forcing RenderSystem Vulkan" << Log::OutS;
		}
#endif
	}

	if (ShouldRunCook)
	{
		IsCooking = true;
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

bool Engine::GetWindowValid() const
{
	return isWindowVaild;
}

void Engine::Exit(int code)
{
	Log::LogMessage("Exit code " + std::to_string(code));
	PlatformWindow::DestroyApplication();
	if (EngineInstance != nullptr)
	{
		EngineInstance->Destory();
		delete EngineInstance;
	}
	exit(code);
}

void Engine::HandleInput(unsigned int key)
{
	if (UIManager::GetCurrentContext() != nullptr)
	{
		UIManager::GetCurrentContext()->ProcessKeyDown(key);
	}
	else
	{
		Input::instance->ProcessKeyDown((unsigned int)key);
	}
}

bool Engine::GetIsCooking() 
{
	return EngineInstance->IsCooking;
}

Engine * Engine::Get()
{
	if (EngineInstance == nullptr)
	{
		EngineInstance = new Engine();
	}
	return EngineInstance;
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
#if WITH_EDITOR
		m_appwnd = new EditorWindow();
#else 
		m_appwnd = new GameWindow();
#endif 
		isWindowVaild = m_appwnd->CreateRenderWindow(width, height);

		if (!isWindowVaild)
		{
			Log::OutS << "Fatal Error: Window Invalid" << Log::OutS;
		}
	}
}

