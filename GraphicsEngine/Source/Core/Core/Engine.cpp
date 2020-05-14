#include "Engine.h"
#include "Assets/AssetManager.h"
#include "Editor/EditorWindow.h"
#include "Game/Game.h"
#include "Input/Input.h"
#include "Module/ModuleManager.h"
#include "Packaging/Cooker.h"
#include "Performance/PerfManager.h"
#include "Platform/ConsoleVariable.h"
#include "Platform/PlatformCore.h"

#include "UI/Core/UIWidget.h"
#include "UI/UIManager.h"
#include "Version.h"
#include "Audio/AudioEngine.h"
#include "AI/Core/AISystem.h"
#include "GameWindow.h"
#include <thread>
#include "Platform/Threading.h"
#include "WinLauncher.h"
#include "Rendering/Core/Defaults.h"

#include "Testing/EngineTests.h"
#include "Rendering/Core/Screen.h"
#include "Module/GameModuleSelector.h"
#include "CSharpInterOp/ICSharpContainerModule.h"
#include "CSharpInterOp/CSharpContainer.h"
#ifdef PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#include "Shlwapi.h"
#endif

Game* Engine::mgame = nullptr;
PhysicsEngine* Engine::PhysEngine = nullptr;
Engine* Engine::EngineInstance = nullptr;

static ConsoleVariable RunCookVar("cook", 0, ECVarType::LaunchOnly);
static ConsoleVariable UseDX12("dx12", 0, ECVarType::LaunchOnly);
static ConsoleVariable UseVK("vk", 0, ECVarType::LaunchOnly);
PhysicsEngine * Engine::GetPhysEngineInstance()
{
	return PhysEngine;
}

std::string Engine::GetExecutionDir()
{
	return PlatformApplication::GetExecutablePath();
}

Engine::Engine(EnginePersistentData* epd) :
	mwidth(0),
	mheight(0)
{
	EPD = epd;

	EngineInstance = this;
	Log::StartLogger();
	PlatformApplication::Init();
	PerfManager::StartPerfManager();
	PerfManager::Get()->StartSingleActionTimer("Engine Boot");

	Log::OutS << "Starting In " << GetExecutionDir() << Log::OutS;
	Log::OutS << "Loading " << ENGINE_NAME << " version " << Version::GetFullVersionString() << Log::OutS;
#if PHYSX_ENABLED
	Log::OutS << "Running with Physx" << Log::OutS;
#else	
	Log::OutS << "Running with TDSim" << Log::OutS;
#endif
	ModuleManager::Get()->PreLoadModules();
#if !BUILD_SHIPPING
	TestArea();
#endif
}

Engine::~Engine()
{
#ifndef NOCSHARP
	CSharpContainer::ShutDown();
#endif
	AssetManager::ShutDownAssetManager();
	Log::ShutDownLogger();
	TaskGraph->Shutdown();
	SafeDelete(TaskGraph);
}

void Engine::PreInit()
{
#if SUPPORTS_COOK
	if (IsCooking)
	{
		Get()->CookContext = new Cooker();
		if (RunCookVar.HasValue())
		{
			Get()->CookContext->SetPlatform(EPlatforms::Parse(RunCookVar.GetRawValueString()));
		}
		ForcedRenderSystem = GetCookContext()->GetTargetRHI();
	}
#endif
	AssetManager::StartAssetManager();

	PhysEngine = new PhysicsEngine();
	if (PhysEngine != nullptr)
	{
		PhysEngine->InitPhysics();
	}
#if RUNTESTS
	FString::RunFStringTests();
#endif
	AudioEngine::Startup();
	int cpucount = std::thread::hardware_concurrency();
	unsigned int threadsToCreate = std::max((int)1, cpucount - 2);
	TaskGraph = new Threading::TaskGraph(threadsToCreate);
	Log::LogMessage("TaskGraph Created with " + std::to_string(threadsToCreate) + " Threads");
	PlatformMisc::SetCurrnetThreadAffinity(0);
#ifndef NOCSHARP
	CSharpContainer::StartUp();
#endif
	Screen::Get()->Resize(mwidth, mheight);
#if TDSIM_ENABLED
	TestTDPhysics();
#endif
	LoadGame();
	if (IsCooking)
	{
		CreateApplication();
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
	SafeDelete(m_appwnd);
}

void Engine::Destory()
{
	RHI::DestoryContext();
	RHI::DestoryRHI();
	if (PhysEngine != nullptr)
	{
		PhysEngine->CleanupPhysics();
	}
	AISystem::ShutDown();
	AudioEngine::Shutdown();
	ModuleManager::Get()->ShutDown();
	//EPD->BenchTime += fabs((float)(PerfManager::get_nanos() - Engine::StartTime) / 1e6f);
	Log::LogMessage("Engine was active for " + std::to_string(EPD->BenchTime) + "ms ");
	PerfManager::ShutdownPerfManager();
}

void Engine::LoadGame()
{
	GameModule* Gamemodule = ModuleManager::Get()->GetModule<GameModule>(GameModuleSelector::GetGameModuleName());
	ensure(Gamemodule);
	if (Gamemodule == nullptr)
	{
		return;
	}
	Log::LogMessage("Loaded game module " + GameModuleSelector::GetGameModuleName());
	Game* gm = Gamemodule->GetGamePtr();
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
	PlatformWindow::TickSplashWindow(10, "Loading RHI");
	if (ForcedRenderSystem == ERenderSystemType::Limit)
	{
#if 0
		RHI::InitRHI(RenderSystemVulkan);
#else
		RHI::InitRHI(RenderSystemD3D12);
#endif
	}
	else
	{
		RHI::InitRHI(ForcedRenderSystem);
	}
	PlatformWindow::TickSplashWindow(10, "Loading Renderer");
	RHI::InitialiseContext();
	PlatformWindow::TickSplashWindow(10, "Loading Scene");
	//TESTING::RunTests();

	if (!IsCooking)
	{
		CreateApplicationWindow(Screen::GetWindowWidth(), Screen::GetWindowHeight());
	}
	PlatformWindow::TickSplashWindow(-1);
}

void Engine::RunCook()
{
#if SUPPORTS_COOK	
	Get()->CookContext->Execute();
	SafeDelete(Get()->CookContext);
#endif
}

void Engine::SetGame(Game * game)
{
	mgame = game;	
	game->Init();
}

Game * Engine::GetGame()
{
	return mgame;
}



void Engine::ProcessCommandLineInput(FString args, int nCmdShow)
{
	mwidth = 1700;
	mheight = 720;
#ifndef PLATFORM_WINDOWS
	mwidth = 1920;
	mheight = 1080;
#endif
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

bool Engine::GetWindowValid() const
{
	return isWindowVaild;
}

void Engine::RequestExit(int code)
{
	if (code != RestartCode)
	{
		GetEPD()->Restart = false;
	}
	if (PlatformWindow::GetApplication() != nullptr)
	{
		PlatformWindow::Kill(code);
	}
	else
	{
		ImmediateExit(code);
	}
}
//this handles no window cases
void Engine::ImmediateExit(int code)
{
	Log::LogMessage("Exit code " + std::to_string(code));
	PlatformWindow::DestroyApplication();
	if (EngineInstance != nullptr)
	{
		EngineInstance->Destory();
		SafeDelete(EngineInstance);
	}
	exit(code);
}

void Engine::AssertExit(int code)
{
	exit(code);
}

Cooker * Engine::GetCookContext()
{
	return Get()->CookContext;
}
#if !BUILD_SHIPPING
void Engine::TestArea()
{}
#endif

void Engine::HandleInput(unsigned int key)
{
	if (UIManager::GetCurrentContext() != nullptr)
	{
		UIManager::GetCurrentContext()->ProcessKeyDown(key);
	}
	else if (Input::Get() != nullptr)
	{
		Input::Get()->ProcessKeyDown((unsigned int)key);
	}
}
void Engine::HandleKeyUp(unsigned int key)
{
	if (UIManager::GetCurrentContext() != nullptr)
	{
		UIManager::GetCurrentContext()->ProcessKeyUp(key);
	}
	else if (Input::Get() != nullptr)
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
	return EngineInstance;
}

float Engine::GetPhysicsDeltaTime()
{
	//#Physx: Smooth DT to avoid Spikes causing weird physics 
	if (EngineInstance != nullptr)
	{
		return std::min(PerfManager::GetDeltaTime(), GetSettings()->MaxPhysicsTimeStep);
	}
	return 0.0f;
}

float Engine::GetDeltaTime()
{
	if (EngineInstance != nullptr)
	{
		return std::min(PerfManager::GetDeltaTime(), 1.0f / 5.0f);
	}
	return 0.0f;
}

Threading::TaskGraph * Engine::GetTaskGraph()
{
	return Get()->TaskGraph;
}

bool Engine::IsSecondLoad()
{
	return Get()->EPD->launchCount > 0;
}

EnginePersistentData * Engine::GetEPD()
{
	return Get()->EPD;
}

void Engine::CreateApplicationWindow(int width, int height)
{
	if (m_appwnd == nullptr)
	{
		mwidth = width;
		mheight = height;
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


//Temp test for physics collisions
#if TDSIM_ENABLED
#include <TDPhysicsAPI.h>
#include "Test/TDTest.h"
void Engine::TestTDPhysics()
{
	TD::TDTest::TypeTest();

}
#endif


