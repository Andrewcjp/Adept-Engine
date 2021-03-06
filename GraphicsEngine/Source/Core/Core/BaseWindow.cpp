#include "BaseWindow.h"
#include "Engine.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "UI/UIManager.h"
#include "Core/Input/Input.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Game/Game.h"
#include "Core/Assets/ImageIO.h"
#include "Audio/AudioEngine.h"
#include "AI/Core/AISystem.h"
#include "UI/Core/UIWidgetContext.h"
#include "Assets/AssetManager.h"
#include "Platform/ConsoleVariable.h"
#include "Rendering/Core/GPUPerformanceGraph.h"
#include "UI/CompoundWidgets/UIGraph.h"
#include "RHI/RHI.h"
#include "Rendering/VR/HMDManager.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/RenderNodes/RenderGraphSystem.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/RenderGraph.h"
#include "Rendering/Core/Screen.h"
#include "Platform/Windows/WindowsWindow.h"
#include "CSharpInterOp/CSharpContainer.h"
#include "Assets/ShaderCompiler.h"
#include "Assets/Scene.h"
#include "Utils/FileUtils.h"
#include "Input/InputManager.h"
#include "Input/InputController.h"
#include "Assets/SceneSeraliser.h"
#include "Performance/HardwareReporter.h"
static ConsoleVariable ShowStats("stats", 0, ECVarType::ConsoleOnly);
static ConsoleVariable FPSCap("maxfps", 0, ECVarType::ConsoleAndLaunch);
static ConsoleVariable RenderScale("r.renderscale", ECVarType::ConsoleAndLaunch, nullptr, nullptr, std::bind(BaseWindow::SetRenderScale, std::placeholders::_1));
static ConsoleVariable Exitvar("exit", ECVarType::ConsoleAndLaunch, std::bind(Engine::RequestExit, 0));
BaseWindow* BaseWindow::Instance = nullptr;
BaseWindow::BaseWindow()
{
	ensure(Instance == nullptr);
	Instance = this;
	RenderScale.SetValueF(RHI::GetRenderSettings()->GetCurrentRenderScale());
}

BaseWindow::~BaseWindow()
{
	Instance = nullptr;
}

bool BaseWindow::CreateRenderWindow(int width, int height)
{
	RHI::InitialiseContextWindow(width, height);
	InitilseWindow();

	PostInitWindow(width, height);
	return true;
}

void BaseWindow::InitilseWindow()
{

	Log::OutS << "Scene Load started" << Log::OutS;
	ImageIO::StartLoader();
	MeshLoader::Get();
	SceneRenderer::StartUp();
	RHI::GetRenderSystem()->InitGraph();
	UI = new UIManager();
	UI->Init(Screen::GetWindowWidth(), Screen::GetWindowHeight());

	Log::LogMessage("Scene initialized");
	LineDrawer = new DebugLineDrawer();
	Saver = new SceneSeraliser();
	GPUPerfGraph = new GPUPerformanceGraph();
	GPUPerfGraph->TwoDrawer = UI->Graph->LineBatcher;

	PerfManager::Get()->AddTimer("Render", "Render");
	PerfManager::Get()->AddTimer("UI", "Render");
	PerfManager::Get()->AddTimer("LineDrawer", "Render");
	PerfManager::Get()->AddTimer("TEXT", "Render");
	if (RHI::GetRenderSettings()->RaytracingEnabled())
	{
		RayTracingEngine::Get()->OnFirstFrame();
	}
	HardwareReporter::Report();
}

void BaseWindow::FixedUpdate()
{

}

void BaseWindow::Render()
{
	PlatformWindow::DestorySplashWindow();
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->ClearStats();
	}
	RHI::BeginFrame();
	if (RHI::SupportVR())
	{
		RHI::GetHMDManager()->Update();
	}
	ShaderCompiler::Get()->TickMaterialCompile();
	PreRender();
	if (PerfManager::Instance != nullptr)
	{
		DeltaTime = PerfManager::GetDeltaTime();
		PerfManager::Instance->StartCPUTimer();
		PerfManager::Instance->StartFrameTimer();
	}
	if (LoadText)
	{
		TextRenderer::instance->Reset();
	}
	AccumTickTime += DeltaTime;
	Input::Get()->ProcessInput();
	AudioEngine::ProcessAudio();
	if (IsRunning)
	{
		AccumTickTime = 0;
		PerfManager::StartTimer("FTick");
		float TimeStep = Engine::GetPhysicsDeltaTime();
		if (TimeStep > 0 && !IsScenePaused())
		{
			{
				SCOPE_CYCLE_COUNTER("stepPhysics");
				Engine::PhysEngine->StepPhysics(TimeStep);
			}
			if (ShouldTickScene)
			{
				CurrentScene->FixedUpdateScene(TimeStep);
			}
			FixedUpdate();
		}
		else if (!IsScenePaused())
		{
			Log::LogMessage("Delta Time was Negative", Log::Severity::Warning);
		}
		PerfManager::EndTimer("FTick");
	}
	if (Input::Get()->GetKeyDown(KeyCode::F11))
	{
		RHI::ToggleFullScreenState();
	}
	if (Input::Get()->GetKeyDown(KeyCode::F9))
	{
		RHI::GetRHIClass()->TriggerBackBufferScreenShot();
	}
	if (Input::Get()->GetKeyDown(KeyCode::F6))
	{
#if ALLOW_RESOURCE_CAPTURE
		RHI::GetRHIClass()->TriggerWriteBackResources();
#endif
	}
	InputController* con = Input::GetInputManager()->GetController(0);

	if (Input::GetKeyDown(KeyCode::F1) || (con != nullptr && con->GetButtonDown(GamePadButtons::FaceButtonRight)))
	{
		ShowText = !ShowText;
	}

	if (Input::GetKeyDown(KeyCode::F2) || (con != nullptr && con->GetButtonDown(GamePadButtons::FaceButtonUp)))
	{
		ShowStats.SetValue(!ShowStats.GetBoolValue());
	}

	if (Input::GetKeyDown(KeyCode::F3) || (con != nullptr && con->GetButtonDown(GamePadButtons::FaceButtonLeft)))
	{
		GPUPerfGraph->SetEnabled(!GPUPerfGraph->IsEnabled());
	}

	if (Input::GetKeyDown(KeyCode::F8))
	{
		SetPauseState(!PauseState);
	}
	if (StepOnce)
	{
		PauseState = true;
		StepOnce = false;
	}
	if (Input::GetKeyDown(KeyCode::F7) && PauseState)
	{
		PauseState = false;
		StepOnce = true;
	}
	if (Input::GetKeyDown(KeyCode::O))
	{
		ConsoleVariableManager::ToggleVar("vrr.showrate");
	}
	if (RHI::GetFrameCount() == 100)
	{
		//LogPerfCounters();
		//RHI::GetRHIClass()->TriggerBackBufferScreenShot();
	}
#if !WITH_EDITOR
	if (Input::GetKeyDown(KeyCode::Escape))
	{
		SetPauseState(true);
	}
#endif
#ifndef NOCSHARP
	CSharpContainer::Update(Engine::GetDeltaTime());
#endif
	Update();
#if !WITH_EDITOR
	if (ShouldTickScene)
	{
		CurrentScene->AlwaysUpdate(DeltaTime);
		if (!IsScenePaused())
		{
			Engine::GetGame()->Update();
			PerfManager::StartTimer("Scene Update");
			CurrentScene->UpdateScene(DeltaTime);
			PerfManager::EndTimer("Scene Update");
		}
	}
#endif
	RHI::Tick();
	PerfManager::StartTimer("Render");
	if (RHI::GetRenderSettings()->RaytracingEnabled() && RHI::GetFrameCount() != 0)
	{
		//RayTracingEngine::Get()->BuildStructures();
	}
	DebugLineDrawer::Get2()->GenerateLines();
	DebugLineDrawer::Get()->GenerateLines();


	RHI::RHIRunFirstFrame();
	RHI::GetRenderSystem()->Update();
	SceneRenderer::Get()->PrepareSceneForRender();
	GPUPerfGraph->Render();
	RHI::GetRenderSystem()->Render();

	PerfManager::EndTimer("Render");
	PerfManager::StartTimer("UI");
	if (UI != nullptr)
	{
		UI->UpdateWidgets();
	}
	if (UI != nullptr && ShowHud && LoadText)
	{
		//UI->RenderWidgets();
	}
	PerfManager::StartTimer("TEXT");
	if (UI != nullptr && ShowHud && LoadText)
	{
		//UI->RenderWidgetText();
	}
	if (LoadText)
	{
		RenderText();
		WindowUI();
	}
	if (LoadText)
	{
		TextRenderer::instance->Finish(true);
	}
	PerfManager::EndTimer("TEXT");
	PerfManager::EndTimer("UI");

	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->EndCPUTimer();
	}
#if !NO_GEN_CONTEXT
	RHI::RHISwapBuffers();
#endif

	if (Once)
	{
		PostFrameOne();
		Once = false;
	}
	Input::Get()->Clear();//clear key states
	PostRender();
	if (TextRenderer::instance != nullptr)
	{
		TextRenderer::instance->NotifyFrameEnd();
	}
	if (CurrentScene != nullptr)
	{
		CurrentScene->OnFrameEnd();
	}
	PerfManager::NotifyEndOfFrame();
	//frameRate limit
	if (FPSCap.GetIntValue() != 0)
	{
		TargetDeltaTime = 1.0f / (FPSCap.GetIntValue() + 1);
		//in MS
		const double WaitTime = std::max((TargetDeltaTime)-(PerfManager::GetDeltaTime()), 0.0)*1000.0f;
		double WaitEndTime = PlatformApplication::Seconds() + (WaitTime / 1000.0);
		if (WaitTime > 0)
		{
			if (WaitTime > 5)
			{
				//Offset a little to give slack to the scheduler
				PlatformApplication::Sleep(WaitTime - 2.0f);
			}
			//spin wait until our time
			while (PlatformApplication::Seconds() < WaitEndTime)
			{
				PlatformApplication::Sleep(0);
			}
		}
	}
	PerfManager::NotifyEndOfFrame(true);
	if (RestartNextFrame)
	{
		ReLoadCurrentScene();
		RestartNextFrame = false;
	}
}

Scene * BaseWindow::GetScene()
{
	return Instance->GetCurrentScene();
}

void BaseWindow::SetPauseState(bool State)
{
	PauseState = State;
	if (PauseState)
	{
		Input::SetCursorState(false, true);
	}
	else
	{
		Input::Get()->ForceClear();
	}
}

void BaseWindow::OnWindowContextLost()
{
	PauseState = true;
}

void BaseWindow::EnqueueRestart()
{
	RestartNextFrame = true;
	for (int i = 0; i < UI->GetContexts().size(); i++)
	{
		UI->GetContexts()[i]->DisplayLoadingScreen();
	}
}

void BaseWindow::SetFrameRateLimit(int limit)
{
	FPSCap.SetValue(limit);
}

void BaseWindow::SetRenderScale(float V)
{
	RHI::GetRenderSettings()->SetRenderScale(glm::clamp(V, 0.1f, 5.0f));
}

void BaseWindow::ReLoadCurrentScene()
{
	CurrentScene->EndScene();
	SceneRenderer::Get()->SetScene(nullptr);
	SafeDelete(CurrentScene);
	CurrentScene = new Scene();
	CurrentScene->LoadExampleScene();
	SceneRenderer::Get()->SetScene(CurrentScene);
	Engine::GetGame()->BeginPlay();
	CurrentScene->StartScene();
	for (int i = 0; i < UI->GetContexts().size(); i++)
	{
		UI->GetContexts()[i]->HideScreen();
	}
}

Scene * BaseWindow::GetCurrentScene()
{
	return CurrentScene;
}

void BaseWindow::LoadScene(std::string RelativePath)
{
	std::string Startdir = Engine::GetExecutionDir();
	Startdir.append(RelativePath);
	SceneRenderer::Get()->SetScene(nullptr);
	SafeDelete(CurrentScene);
	CurrentScene = new Scene();
	if (Saver)
	{
		Saver->LoadScene(CurrentScene, Startdir);
	}
	SceneRenderer::Get()->SetScene(CurrentScene);
}

BaseWindow * BaseWindow::Get()
{
	return Instance;
}

void BaseWindow::PostFrameOne()
{
	float BootTime = PerfManager::Get()->EndSingleActionTimer("Engine Boot");
	PerfManager::Instance->LogSingleActionTimers();
	Log::OutS << "Engine Loaded in " << BootTime << "ms " << Log::OutS;
	//debug output
	std::string stamp = PlatformMisc::GetDateTimeString() + ": boot took " + std::to_string(BootTime) + "ms \n";
	FileUtils::WriteToFile(AssetManager::GetGeneratedDir() + "BootTimes.txt", stamp, true);
}

void BaseWindow::Resize(int width, int height, bool force /*= false*/)
{
	if (!Screen::IsValidForRender())
	{
		return;
	}
	if (UI != nullptr)
	{
		UI->UpdateSize(width, height);
	}
	RHI::GetRenderSystem()->GetCurrentGraph()->Resize();
	Log::LogMessage("Renderer resized to " + std::to_string(Screen::GetScaledWidth()) + " X " + std::to_string(Screen::GetScaledHeight()) + " (~" + std::to_string(Screen::GetScaledHeight()) + "p)");
}

void BaseWindow::DestroyRenderWindow()
{
	RHI::WaitForGPU();
	CurrentScene->EndScene();
	SafeDelete(CurrentScene);
	ImageIO::ShutDown();
	MeshLoader::ShutDown();
	//	SafeDelete(LineDrawer);
	SafeDelete(UI);
	Input::ShutDown();
}

void BaseWindow::StaticResize()
{
	Instance->Resize(Screen::GetWindowWidth(), Screen::GetWindowHeight());
}

void BaseWindow::RenderText()
{
	const int m_height = Screen::GetWindowHeight();
	const int m_width = Screen::GetWindowWidth();
	int offset = 2;
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2);
	if (ShowText)
	{
		stream << PerfManager::Instance->GetAVGFrameRate() << " " << (PerfManager::Instance->GetAVGFrameTime() * 1000) << "ms ";
		stream << "Ratio " << RHI::GetRenderSettings()->GetCurrentRenderScale() << "X ";
		stream << "GPU :" << PerfManager::GetGPUTime() << "ms ";
		stream << "CPU " << std::setprecision(2) << PerfManager::GetCPUTime() << "ms ";
		if (Input::GetKeyDown(KeyCode::P))
		{
			Log::LogMessage(stream.str());
		}
		UI->RenderTextToScreen(1, stream.str());
	}
	stream.str("");
	const bool ExtendedPerformanceStats = ShowStats.GetBoolValue();
	if (ExtendedPerformanceStats)
	{
		if (RHI::GetRHIClass() != nullptr)
		{
			PerfManager::RenderGpuData(10, (int)(m_height - m_height / 7));
		}

		if (PerfManager::Instance != nullptr)
		{
			PerfManager::Instance->DrawAllStats(m_width / 3, (int)(m_height / 1.2));
			PerfManager::Instance->DrawAllStats((int)(m_width / 1.5f), (int)(m_height / 1.2), true);
		}

		PlatformMemoryInfo info = PlatformMisc::GetMemoryInfo();
		UI->RenderTextToScreen(2, RHI::ReportMemory(true) + " CPU ram " + StringUtils::ToString(info.GetWorkingSetInMB()) + "MB");

		RHI::GetRenderSystem()->GetCurrentGraph()->DebugOutput();
		offset = 4;
	}
	if (!RHI::AllowCPUAhead())
	{
		offset++;
		UI->RenderTextToScreen(offset, "AllowCPUAhead Disabled", Colours::RED);
	}
	if (RHI::BlockCommandlistExec())
	{
		offset++;
		UI->RenderTextToScreen(offset, "BlockCommandlistExec Enabled", Colours::RED);
	}
	Log::Get()->RenderText(UI, offset);
	if (Input::GetKeyDown(KeyCode::P))
	{
		LogPerfCounters();
	}
}

void BaseWindow::LogPerfCounters()
{
	std::vector<TimerData*> Data = PerfManager::Instance->GetAllGPUTimers("GPU_0");
	for (int i = 0; i < Data.size(); i++)
	{
		Log::LogMessage(Data[i]->name + " :" + std::to_string(Data[i]->AVG->GetCurrentAverage()));
	}
}
