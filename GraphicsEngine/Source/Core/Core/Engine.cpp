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
#include "Input/Input.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Performance/PerfManager.h"
#include "UI/Core/UIWidget.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Version.h"
#pragma comment(lib, "shlwapi.lib")
#include "Core/Platform/Windows/WindowsWindow.h"
#include "AI/Core/AISystem.h"
#include "Audio/AudioEngine.h"
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
	Log::StartLogger();
	PlatformApplication::Init();
	StartTime = (float)PerfManager::get_nanos();
	Log::OutS << "Starting In " << GetExecutionDir() << Log::OutS;
	Log::OutS << "Loading Engine v" << ENGINE_VERSION << Log::OutS;
#if PHYSX_ENABLED
	Log::OutS << "Running with Physx" << Log::OutS;
#else	
	Log::OutS << "Running with TDSim" << Log::OutS;
#endif
	ModuleManager::Get()->PreLoadModules();
	AssetManager::StartAssetManager();
	PerfManager::StartPerfManager();	
	PhysEngine = new PhysicsEngine();
	if (PhysEngine != nullptr)
	{
		PhysEngine->initPhysics();
	}	
	CompRegistry = new CompoenentRegistry();
#if RUNTESTS
	FString::RunFStringTests();
#endif
	
	AudioEngine::Startup();
}

Engine::~Engine()
{
	Log::ShutDownLogger();
}
void Engine::PreInit()
{	
	TestTDPhysics();
	LoadGame();
	if (IsCooking)
	{
		RunCook();
	}
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
	RHI::DestoryRHI();
	if (PhysEngine != nullptr)
	{
		PhysEngine->cleanupPhysics();
	}
	AISystem::ShutDown();
	AudioEngine::Shutdown();
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
	AISystem::StartUp();
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
}

void Engine::RunCook()
{
	Cooker* cook = new Cooker();
	cook->CopyToOutput();
	SafeDelete(cook);
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
static ConsoleVariable UseDX12("dx12", 0, ECVarType::LaunchOnly);
static ConsoleVariable UseVK("vk", 0, ECVarType::LaunchOnly);

void Engine::ProcessCommandLineInput(FString args, int nCmdShow)
{
	mwidth = 1700;
	mheight = 720;
	if (nCmdShow > 0)
	{
		ConsoleVariableManager::SetupVars(args.ToSString());
		if (RunCookVar.GetBoolValue())
		{
			Log::OutS << "Starting Cook" << Log::OutS;
			ShouldRunCook = true;
		}

		if (UseDX12.GetBoolValue())
		{
			ForcedRenderSystem = RenderSystemD3D12;
			Log::OutS << "Forcing RenderSystem D3D12" << Log::OutS;
		}
#if BUILD_VULKAN
		if (UseVK.GetBoolValue())
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
		Input::Get()->ProcessKeyDown((unsigned int)key);
	}
}
void Engine::HandleKeyUp(unsigned int key)
{
	if (UIManager::GetCurrentContext() != nullptr)
	{
		//UIManager::GetCurrentContext()->ProcessKeyDown(key); TODO: this
	}
	else
	{
		Input::Get()->ProcessKeyUp((unsigned int)key);
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

float Engine::GetPhysicsDeltaTime()
{
	//Todo: Smooth DT to avoid Spikes causing weird physics 
	if (EngineInstance != nullptr)
	{
		return std::min(PerfManager::GetDeltaTime(), GetSettings()->MaxPhysicsTimeStep);
	}
	return 0.0f;
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
#include "Test/TDTest.h"
void Engine::TestTDPhysics()
{
	TD::TDTest::RunAllTests();
}

