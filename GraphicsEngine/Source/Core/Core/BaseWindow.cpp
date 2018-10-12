#include "stdafx.h"
#include "BaseWindow.h"
#include "Engine.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Renderers/DeferredRenderer.h"
#include "Rendering/Renderers/ForwardRenderer.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "RHI/RHI.h"
#include "UI/UIManager.h"
#include "Core/Input/Input.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "RHI/DeviceContext.h"
#include <algorithm>
#include "Core/Platform/PlatformCore.h"
#include "Core/Game/Game.h"
#include "Core/Assets/ImageIO.h"
#include "Audio/AudioEngine.h"
BaseWindow* BaseWindow::Instance = nullptr;
BaseWindow::BaseWindow()
{
	ensure(Instance == nullptr);
	Instance = this;
	RHI::GetRenderSettings()->RenderScale = 1;
}


BaseWindow::~BaseWindow()
{

}

bool BaseWindow::ChangeDisplayMode(int width, int height)
{
#if 0
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	EnumDisplaySettings(NULL, 0, &dmScreenSettings);
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = width;
	dmScreenSettings.dmPelsHeight = height;
	dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

	// Try To Set Selected Mode And Get Results.
	LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	if (result != DISP_CHANGE_SUCCESSFUL)
	{
		//__debugbreak();
		return false;
	}
#endif
	return true;
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
	fprintf(stdout, "Scene initalised\n");
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->SampleNVCounters();
	}
	LineDrawer = new DebugLineDrawer();
	Saver = new SceneJSerialiser();

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
	AccumTickTime += DeltaTime;
	Input::Get()->ProcessInput(DeltaTime);
	AudioEngine::ProcessAudio();
	//lock the simulation rate to TickRate
	//this prevents physx being framerate depenent.
	if (IsRunning)
	{
		AccumTickTime = 0;
		PerfManager::StartTimer("FTick");
		float TimeStep = Engine::GetPhysicsDeltaTime();
		if (TimeStep > 0)
		{
			Engine::PhysEngine->stepPhysics(TimeStep);
			if (ShouldTickScene)
			{
				CurrentScene->FixedUpdateScene(TimeStep);
			}
			FixedUpdate();
		}
		else
		{
			Log::LogMessage("Delta Time was Negative", Log::Severity::Warning);
		}
		//CurrentPlayScene->FixedUpdateScene(TickRate);
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
#endif

	Update();
	if (ShouldTickScene)
	{
		Engine::GetGame()->Update();
		//PerfManager::StartTimer("Scene Update");
		CurrentScene->UpdateScene(DeltaTime);
		//PerfManager::EndTimer("Scene Update");
	}

	PerfManager::StartTimer("Render");
	Renderer->Render();

	LineDrawer->GenerateLines();
	if (Renderer->GetMainCam() != nullptr)
	{
		LineDrawer->RenderLines(Renderer->GetMainCam()->GetViewProjection());
	}

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
	TextRenderer::instance->Reset();
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
	if (FrameRateLimit != 0)
	{
		TargetDeltaTime = 1.0f / (FrameRateLimit + 1);
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
	PerfManager::NotifyEndOfFrame();
}

bool BaseWindow::ProcessDebugCommand(std::string command)
{
	if (Instance != nullptr)
	{
		if (command.find("stats") != -1)
		{
			if (PerfManager::Instance != nullptr)
			{
				PerfManager::Instance->ShowAllStats = !PerfManager::Instance->ShowAllStats;
			}
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

Scene * BaseWindow::GetCurrentScene()
{
	return CurrentScene;
}

void BaseWindow::LoadScene(std::string RelativePath)
{
	std::string Startdir = Engine::GetExecutionDir();
	Startdir.append(RelativePath);
	Renderer->SetScene(nullptr);
	delete CurrentScene;
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
	ImageIO::ShutDown();
	Renderer->DestoryRenderWindow();
	delete LineDrawer;
	SafeDelete(UI);
	SafeDelete(Renderer);
	CurrentScene->EndScene();
	delete CurrentScene;
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
	if (!ShowText)
	{
		return;
	}
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
		/*stream << RHI::GetRHIClass()->GetMemory();
		UI->RenderTextToScreen(2, stream.str());*/
		PerfManager::RenderGpuData(10, (int)(m_height - m_height / 8));
	}

	if (PerfManager::Instance != nullptr && ExtendedPerformanceStats)
	{
		PerfManager::Instance->DrawAllStats(m_width / 2, (int)(m_height / 1.2));
	}
}
