#include "BaseWindow.h"
#include "Engine.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Renderers/DeferredRenderer.h"
#include "Rendering/Renderers/ForwardRenderer.h"
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

static ConsoleVariable FPSCap("maxfps", 60, ECVarType::ConsoleAndLaunch);
BaseWindow* BaseWindow::Instance = nullptr;
BaseWindow::BaseWindow()
{
	ensure(Instance == nullptr);
	Instance = this;
}

BaseWindow::~BaseWindow()
{

}

bool BaseWindow::CreateRenderWindow(int width, int height)
{
	RHI::InitialiseContextWindow(width, height);
	Material::SetupDefaultMaterial();//move!
	m_height = height;
	m_width = width;
	InitilseWindow();

	PostInitWindow(width, height);
	return true;
}

void BaseWindow::InitilseWindow()
{
	Log::OutS << "Scene Load started" << Log::OutS;
	ImageIO::StartLoader();
	MeshLoader::Get();
	if (RHI::GetRenderSettings()->IsDeferred)
	{
		Renderer = new DeferredRenderer(m_width, m_height);
	}
	else
	{
		Renderer = new ForwardRenderer(m_width, m_height);
	}
	Renderer->Init();

	UI = new UIManager(m_width, m_height);
	Input::Startup();
	Log::LogMessage("Scene initialized");
	LineDrawer = new DebugLineDrawer();
	Saver = new SceneJSerialiser();
	std::vector<std::string> PreLoadTextures = Engine::GetGame()->GetPreLoadAssets();
	for (int i = 0; i < PreLoadTextures.size(); i++)
	{
		AssetManager::DirectLoadTextureAsset(PreLoadTextures[i]);
	}
}

void BaseWindow::FixedUpdate()
{

}

void BaseWindow::Render()
{
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->ClearStats();
	}
	PreRender();
	if (PerfManager::Instance != nullptr)
	{
		DeltaTime = PerfManager::GetDeltaTime();
		PerfManager::Instance->StartCPUTimer();
		PerfManager::Instance->StartFrameTimer();
	}
	TextRenderer::instance->Reset();
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

#if 1
	/*if (input->GetKeyDown(VK_ESCAPE))
	{
		PostQuitMessage(0);
	}*/
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
		RHI::GetRHIClass()->TriggerWriteBackResources();
	}
	if (Input::GetKeyDown(VK_F1))
	{
		ShowText = !ShowText;
	}
	if (Input::GetKeyDown(VK_F2))
	{
		ExtendedPerformanceStats = !ExtendedPerformanceStats;
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

	PerfManager::StartTimer("Render");
	Renderer->Render();
	PerfManager::StartTimer("LineDrawer");
	LineDrawer->GenerateLines();
	if (Renderer->GetMainCam() != nullptr)
	{
		LineDrawer->RenderLines(Renderer->GetMainCam()->GetViewProjection());
	}
	PerfManager::EndTimer("LineDrawer");
	Renderer->FinaliseRender();
	PerfManager::EndTimer("Render");
	PerfManager::StartTimer("UI");
	if (UI != nullptr)
	{
		UI->UpdateWidgets();
	}
	if (UI != nullptr && ShowHud && LoadText)
	{
		UI->RenderWidgets();
	}
	if (PostProcessing::Instance)
	{
		PostProcessing::Instance->ExecPPStackFinal(nullptr);
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
	PerfManager::EndTimer("TEXT");
	TextRenderer::instance->Finish();

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
		double LastTime = PlatformApplication::Seconds();
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

bool BaseWindow::ProcessDebugCommand(std::string command, std::string & response)
{
	if (Instance != nullptr)
	{
		/*	if (command.find("fps") != -1)
			{
				Instance->ShowText = !Instance->ShowText;
				return true;
			}
			else */
		if (command.find("stats") != -1)
		{
			Instance->ExtendedPerformanceStats = !Instance->ExtendedPerformanceStats;
			return true;
		}
		else if (command.find("renderscale") != -1)
		{
			StringUtils::RemoveChar(command, "renderscale");
			StringUtils::RemoveChar(command, " ");
			if (command.length() > 0)
			{
				RHI::GetRenderSettings()->RenderScale = glm::clamp(stof(command), 0.1f, 5.0f);
				Instance->Resize(Instance->m_width, Instance->m_height);
			}
			response = "renderscale " + std::to_string(RHI::GetRenderSettings()->RenderScale);
			return true;
		}
		else if (command.find("exit") != -1)
		{
			Engine::RequestExit(0);
			return true;
		}
	}
	return false;
}

Camera * BaseWindow::GetCurrentCamera()
{
	if (Instance != nullptr && Instance->Renderer != nullptr)
	{
		return Instance->Renderer->GetMainCam();
	}
	return nullptr;
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

void BaseWindow::ReLoadCurrentScene()
{
	CurrentScene->EndScene();
	Renderer->SetScene(nullptr);
	SafeDelete(CurrentScene);
	CurrentScene = new Scene();
	CurrentScene->LoadExampleScene(nullptr, false);
	Renderer->SetScene(CurrentScene);
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
	Renderer->SetScene(nullptr);
	SafeDelete(CurrentScene);
	CurrentScene = new Scene();
	if (Saver)
	{
		Saver->LoadScene(CurrentScene, Startdir);
	}
	Renderer->SetScene(CurrentScene);
}

void BaseWindow::PostFrameOne()
{
	PerfManager::Instance->LogSingleActionTimers();
	Log::OutS << "Engine Loaded in " << fabs((PerfManager::get_nanos() - Engine::StartTime) / 1e6f) << "ms " << Log::OutS;
}

void BaseWindow::Resize(int width, int height)
{
	if (width == m_width && height == m_height || width == 0 || height == 0)
	{
		return;
	}
	m_width = width;
	m_height = height;
	if (UI != nullptr)
	{
		UI->UpdateSize(width, height);
	}
	if (Renderer != nullptr)
	{
		RHI::ResizeSwapChain(width, height);
		Renderer->Resize(width, height);
	}
}

void BaseWindow::DestroyRenderWindow()
{
	RHI::WaitForGPU();
	CurrentScene->EndScene();
	SafeDelete(CurrentScene);
	ImageIO::ShutDown();
	MeshLoader::ShutDown();
	Renderer->DestoryRenderWindow();
	SafeDelete(LineDrawer);
	SafeDelete(UI);
	SafeDelete(Renderer);
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

//getters
int BaseWindow::GetWidth()
{
	if (Instance != nullptr)
	{
		return Instance->m_width;
	}
	return 0;
}

int BaseWindow::GetHeight()
{
	if (Instance != nullptr)
	{
		return Instance->m_height;
	}
	return 0;
}

RenderEngine * BaseWindow::GetCurrentRenderer()
{
	return Renderer;
}

void BaseWindow::RenderText()
{
	int offset = 1;
	if (ShowText)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2);
		stream << PerfManager::Instance->GetAVGFrameRate() << " " << (PerfManager::Instance->GetAVGFrameTime() * 1000) << "ms " << Engine::GetPhysicsDeltaTime() * 1000 << "ms ";
		if (RHI::GetRenderSettings()->IsDeferred)
		{
			stream << "DEF ";
		}
		stream << "GPU :" << PerfManager::GetGPUTime() << "ms ";
		stream << "CPU " << std::setprecision(2) << PerfManager::GetCPUTime() << "ms ";
		UI->RenderTextToScreen(1, stream.str());
		stream.str("");

		if (RHI::GetRHIClass() != nullptr && ExtendedPerformanceStats)
		{
			PerfManager::RenderGpuData(10, (int)(m_height - m_height / 8));
		}

		if (PerfManager::Instance != nullptr && ExtendedPerformanceStats)
		{
			PerfManager::Instance->DrawAllStats(m_width / 3, (int)(m_height / 1.2));
			PerfManager::Instance->DrawAllStats((int)(m_width / 1.5f), (int)(m_height / 1.2), true);
		}
		if (ExtendedPerformanceStats)
		{
			UI->RenderTextToScreen(2, RHI::ReportMemory());
		}
		offset = 3;
	}
	Log::Get()->RenderText(UI, offset);
}
