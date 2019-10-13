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
#include "UI/GameUI/UIGraph.h"
#include "RHI/RHI.h"
#include "Rendering/VR/HMDManager.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/RenderNodes/RenderGraphSystem.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/RenderGraph.h"
#include "Rendering/Core/Screen.h"
#include "Platform/Windows/WindowsWindow.h"
#include "CSharpInterOp/CSharpContainer.h"
#include "Assets/ShaderComplier.h"
#include "Assets/Scene.h"
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
#if !BASIC_RENDER_ONLY
	UI = new UIManager(Screen::GetWindowWidth(), Screen::GetWindowHeight());
#endif

	Log::LogMessage("Scene initialized");
	LineDrawer = new DebugLineDrawer();
	Saver = new SceneJSerialiser();
	std::vector<std::string> PreLoadTextures = Engine::GetGame()->GetPreLoadAssets();
	for (int i = 0; i < PreLoadTextures.size(); i++)
	{
		AssetManager::DirectLoadTextureAsset(PreLoadTextures[i]);
	}
#if !BASIC_RENDER_ONLY
	GPUPerfGraph = new GPUPerformanceGraph();
	GPUPerfGraph->TwoDrawer = UI->Graph->LineBatcher;
#endif
	PerfManager::Get()->AddTimer("Render", "Render");
	PerfManager::Get()->AddTimer("UI", "Render");
	PerfManager::Get()->AddTimer("LineDrawer", "Render");
	PerfManager::Get()->AddTimer("TEXT", "Render");
	if (RHI::GetRenderSettings()->RaytracingEnabled())
	{
		RayTracingEngine::Get()->OnFirstFrame();
	}
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
	ShaderComplier::Get()->TickMaterialComplie();
	PreRender();
	if (PerfManager::Instance != nullptr)
	{
		DeltaTime = PerfManager::GetDeltaTime();
		PerfManager::Instance->StartCPUTimer();
		PerfManager::Instance->StartFrameTimer();
	}
#if !BASIC_RENDER_ONLY
	if (LoadText)
	{
		TextRenderer::instance->Reset();
	}
#endif
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
				//Engine::PhysEngine->StepPhysics(TimeStep);
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
	if (Input::Get()->GetKeyDown(VK_F11))
	{
		RHI::ToggleFullScreenState();
	}
	if (Input::Get()->GetKeyDown(VK_F9))
	{
		RHI::GetRHIClass()->TriggerBackBufferScreenShot();
	}
	if (Input::Get()->GetKeyDown(VK_F6))
	{
#if ALLOW_RESOURCE_CAPTURE
		RHI::GetRHIClass()->TriggerWriteBackResources();
#endif
}
	if (Input::GetKeyDown(VK_F1))
	{
		ShowText = !ShowText;
	}
	if (Input::GetKeyDown(VK_F2))
	{
		ShowStats.SetValue(!ShowStats.GetBoolValue());
	}
	if (Input::GetKeyDown(VK_F3))
	{
		GPUPerfGraph->SetEnabled(!GPUPerfGraph->IsEnabled());
	}

	if (Input::GetKeyDown(VK_F8))
	{
		SetPauseState(!PauseState);
	}
	if (StepOnce)
	{
		PauseState = true;
		StepOnce = false;
	}
	if (Input::GetKeyDown(VK_F7) && PauseState)
	{
		PauseState = false;
		StepOnce = true;
	}
#if !WITH_EDITOR
	if (Input::GetKeyDown(VK_ESCAPE))
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
		//if (!IsScenePaused())
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
#if !BASIC_RENDER_ONLY
	if (RHI::GetRenderSettings()->RaytracingEnabled() && RHI::GetFrameCount() != 0)
	{
		//RayTracingEngine::Get()->BuildStructures();
	}
	DebugLineDrawer::Get2()->GenerateLines();
	DebugLineDrawer::Get()->GenerateLines();
#endif

	RHI::RHIRunFirstFrame();
	RHI::GetRenderSystem()->Update();
	SceneRenderer::Get()->PrepareSceneForRender();
	RHI::GetRenderSystem()->Render();

#if !BASIC_RENDER_ONLY
	PerfManager::EndTimer("Render");
	PerfManager::StartTimer("UI");
	GPUPerfGraph->Render();
	if (UI != nullptr)
	{
		UI->UpdateWidgets();
	}
	if (UI != nullptr && ShowHud && LoadText)
	{
		UI->RenderWidgets();
	}
	PerfManager::StartTimer("TEXT");
	if (UI != nullptr && ShowHud && LoadText)
	{
		UI->RenderWidgetText();
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
#endif
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
	PerfManager::Instance->LogSingleActionTimers();
	Log::OutS << "Engine Loaded in " << fabs((PerfManager::get_nanos() - Engine::StartTime) / 1e6f) << "ms " << Log::OutS;
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
	SafeDelete(LineDrawer);
	SafeDelete(UI);
	Input::ShutDown();
}

bool BaseWindow::MouseLBDown(int x, int y)
{
	if (UI != nullptr)
	{
		UI->MouseClick(x, y);
	}
	return true;
}

bool BaseWindow::MouseLBUp(int x, int y)
{
	if (UI != nullptr)
	{
		UI->MouseClickUp(x, y);
	}
	return true;
}

bool BaseWindow::MouseRBDown(int x, int y)
{
	if (UI != nullptr)
	{
		if (!UI->IsUIBlocking())
		{
			Input::Get()->MouseLBDown(x, y);
		}
	}
	else
	{
		Input::Get()->MouseLBDown(x, y);
	}
	return 0;
}

bool BaseWindow::MouseRBUp(int x, int y)
{
	if (UI)
	{
		if (!UI->IsUIBlocking())
		{
			Input::Get()->MouseLBUp(x, y);
		}
	}
	else
	{
		Input::Get()->MouseLBUp(x, y);
	}
	return 0;
}

bool BaseWindow::MouseMove(int x, int y)
{
	Input::Get()->MouseMove(x, y, DeltaTime);
	if (UI != nullptr)
	{
		UI->MouseMove(x, y);
	}
	return true;
}

void BaseWindow::StaticResize()
{
	Instance->Resize(Screen::GetWindowWidth(), Screen::GetWindowHeight());
}

void BaseWindow::RenderText()
{
	const int m_height = Screen::GetWindowHeight();
	const int m_width = Screen::GetWindowWidth();
	int offset = 1;
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2);
	if (ShowText)
	{
		stream << PerfManager::Instance->GetAVGFrameRate() << " " << (PerfManager::Instance->GetAVGFrameTime() * 1000) << "ms ";
		stream << "Ratio " << RHI::GetRenderSettings()->GetCurrentRenderScale() << "X ";
		stream << "GPU :" << PerfManager::GetGPUTime() << "ms ";
		stream << "CPU " << std::setprecision(2) << PerfManager::GetCPUTime() << "ms ";
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
		UI->RenderTextToScreen(2, RHI::ReportMemory() + " CPU ram " + StringUtils::ToString(info.GetWorkingSetInMB()) + "MB");

		RHI::GetRenderSystem()->GetCurrentGraph()->DebugOutput();
		offset = 4;

	}

	Log::Get()->RenderText(UI, offset);
}
