#include "stdafx.h"
#include "BaseWindow.h"
#include <mutex>
#include <thread>
#include "Engine.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Renderers/DeferredRenderer.h"
#include "Rendering/Renderers/ForwardRenderer.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "RHI/RHI.h"
#include "UI/UIManager.h"
#include "Core/Input.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Core/Assets/ImageIO.h"
#include "RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "RHI/DeviceContext.h"
#include <algorithm>
#include "Core/Platform/WindowsApplication.h"
BaseWindow* BaseWindow::Instance = nullptr;
BaseWindow::BaseWindow()
{
	assert(Instance == nullptr);
	Instance = this;
	WindowsApplication::InitTiming();
	CurrentRenderSettings.RenderScale = 1;
}


BaseWindow::~BaseWindow()
{

}

bool BaseWindow::ChangeDisplayMode(int width, int height)
{

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

	return true;
}

bool BaseWindow::CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen)
{
	//window manager class?
	if (Fullscreen)
	{
		if (ChangeDisplayMode(width, height))
		{
			m_hwnd = CreateWindowEx(NULL,
				L"RenderWindow", L"OGLWindow", WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				0, 0, width, height, NULL, NULL, hInstance, NULL);
		}
	}
	else
	{
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
			L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, width, height, NULL, NULL, hInstance, NULL);
	}
	m_hInstance = hInstance;
	RHI::instance->m_hinst = hInstance;
	RHI::InitialiseContext(m_hwnd, width, height);
	m_height = height;
	m_width = width;
	InitilseWindow();
	PostInitWindow(width, height);
	return true;
}
void BaseWindow::InitilseWindow()
{
	Log::OutS  << "Scene Load started" << Log::OutS;
	ImageIO::StartLoader();
	IsDeferredMode = true; 
	if (IsDeferredMode)
	{
		Renderer = new DeferredRenderer(m_width, m_height);
	}
	else
	{
		Renderer = new ForwardRenderer(m_width, m_height);
	}
	Renderer->SetRenderSettings(CurrentRenderSettings);
	Renderer->Init();
	CurrentScene = new Scene();
	CurrentScene->LoadDefault();
	Renderer->SetScene(CurrentScene);
	UI = new UIManager(m_width, m_height);
	input = new Input(m_hwnd);
	input->Cursor = CopyCursor(LoadCursor(NULL, IDC_ARROW));
	input->Cursor = SetCursor(input->Cursor);

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
		if (FrameRateLimit == 0)
		{
			DeltaTime = PerfManager::GetDeltaTime();
		}
		PerfManager::Instance->StartCPUTimer();
		PerfManager::Instance->StartFrameTimer();
	}
	AccumTickTime += DeltaTime;
	input->ProcessInput(DeltaTime);
	input->ProcessQue();

	//lock the simulation rate to TickRate
	//this prevents physx being framerate depenent.
	if (AccumTickTime > TickRate && IsRunning)
	{
		AccumTickTime = 0;
		PerfManager::StartTimer("FTick");
		Engine::PhysEngine->stepPhysics(TickRate);
		if (ShouldTickScene)
		{
			CurrentScene->FixedUpdateScene(TickRate);
		}
		FixedUpdate();
		//CurrentPlayScene->FixedUpdateScene(TickRate);
		PerfManager::EndTimer("FTick");
	}
#if 1
	if (input->GetKeyDown(VK_ESCAPE))
	{
		PostQuitMessage(0);
	}
	if (input->GetKeyDown(VK_F11))
	{
		//D3D12RHI::Instance->ToggleFullScreenState();
		RHI::ToggleFullScreenState();
	}
#endif
	Update();
	if (ShouldTickScene)
	{
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

	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->SampleNVCounters();
		PerfManager::Instance->EndFrameTimer();
	}

	if (Once)
	{
		PostFrameOne();
		Once = false;
	}
	input->Clear();//clear key states
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
		TargetDeltaTime = 1.0f / FrameRateLimit;
		//in MS
		const double WaitTime = std::max((TargetDeltaTime*1000.0) - (DeltaTime), 0.0);
		double WaitEndTime = WindowsApplication::Seconds() + (WaitTime / 1000.0);
		double LastTime = WindowsApplication::Seconds();
		if (WaitTime > 0)
		{
			if (WaitTime > 5 / 1000.0)
			{
				//little offset
				WindowsApplication::Sleep(WaitTime);
			}

			while (WindowsApplication::Seconds() < WaitEndTime)
			{
				WindowsApplication::Sleep(0);
			}
			DeltaTime = WindowsApplication::Seconds() - LastTime;
			PerfManager::SetDeltaTime(DeltaTime);
		}
	}
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
				Instance->CurrentRenderSettings.RenderScale = glm::clamp(stof(command), 0.1f, 5.0f);
				Instance->Renderer->SetRenderSettings(Instance->CurrentRenderSettings);
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

void BaseWindow::LoadScene(std::string RelativePath)
{
	std::string Startdir = Engine::GetRootDir();
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
	Log::OutS  << "Engine Loaded in " << fabs((PerfManager::get_nanos() - Engine::StartTime) / 1e6f) << "ms " << Log::OutS;
#if BUILD_D3D12 && !USEGPUTOGENMIPS_ATRUNTIME
	if (RHI::GetType() == RenderSystemD3D12)
	{
		Log::OutS  << "MipMaps took " << D3D12Texture::MipCreationTime << "MS to generate" << Log::OutS;
	}
#endif

}
void BaseWindow::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (UI != nullptr)
	{
		UI->UpdateSize(width, height);
	}
	if (Renderer != nullptr)
	{
		Renderer->Resize(width, height);
	}
}

void BaseWindow::DestroyRenderWindow()
{
	D3D12RHI::Instance->WaitForAllDevices();
	ImageIO::ShutDown();
	Renderer->DestoryRenderWindow();
	delete input;
	delete LineDrawer;
	delete UI;	
	delete Renderer;
	delete CurrentScene;
	RHI::DestoryContext(m_hwnd);
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;
}

BOOL BaseWindow::MouseLBDown(int x, int y)
{
	if (UI != nullptr)
	{
		UI->MouseClick(x, y);
	}
	return TRUE;
}

BOOL BaseWindow::MouseLBUp(int x, int y)
{
	if (UI != nullptr)
	{
		UI->MouseClickUp(x, y);
	}
	return TRUE;
}

BOOL BaseWindow::MouseRBDown(int x, int y)
{
	if (UI != nullptr)
	{
		if (!UI->IsUIBlocking())
		{
			input->MouseLBDown(x, y);
		}
	}
	else
	{
		input->MouseLBDown(x, y);
	}

	return 0;
}

BOOL BaseWindow::MouseRBUp(int x, int y)
{
	if (UI)
	{
		if (!UI->IsUIBlocking())
		{
			input->MouseLBUp(x, y);
		}
	}
	else
	{
		input->MouseLBUp(x, y);
	}

	return 0;
}

BOOL BaseWindow::MouseMove(int x, int y)
{
	input->MouseMove(x, y, DeltaTime);
	if (UI != nullptr)
	{
		UI->MouseMove(x, y);
	}
	return TRUE;
}

BOOL BaseWindow::KeyDown(WPARAM key)
{
	if (UIManager::GetCurrentContext() != nullptr)
	{
		UIManager::GetCurrentContext()->ProcessKeyDown(key);
	}
	else
	{
		input->ProcessKeyDown(key);
	}
	return TRUE;
}

void BaseWindow::ProcessMenu(WORD command)
{}

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
	stream << PerfManager::Instance->GetAVGFrameRate() << " " << (PerfManager::Instance->GetAVGFrameTime() * 1000) << "ms ";
	if (IsDeferredMode)
	{
		stream << "DEF ";
	}
	stream << "GPU :" << PerfManager::GetGPUTime() << "ms ";
	stream << "CPU " << std::setprecision(2) << PerfManager::GetCPUTime() << "ms ";

	UI->RenderTextToScreen(1, stream.str());
	stream.str("");
	if (D3D12RHI::Instance != nullptr && ExtendedPerformanceStats)
	{
		stream << D3D12RHI::Instance->GetMemory();
		UI->RenderTextToScreen(2, stream.str());
	}

	if (PerfManager::Instance != nullptr && ExtendedPerformanceStats)
	{
		PerfManager::Instance->DrawAllStats(m_width / 2, (int)(m_height / 1.2));
	}
}
