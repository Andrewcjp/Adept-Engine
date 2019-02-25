#include "Source/BleedOutGame/BleedOutPCH.h"
#include "LevelBenchMarker.h"
#include "Core/Performance/PerfManager.h"
#include "Core/BaseWindow.h"
#include "Core/Engine.h"
#include "GraphicsEngine.h"
#include "Rendering/Renderers/RenderSettings.h"


LevelBenchMarker::LevelBenchMarker()
{
	PreHeatTime = 3.0f;
	PreHeatTimer = PreHeatTime;
}

LevelBenchMarker::~LevelBenchMarker()
{}

void LevelBenchMarker::AddAllRes(MGPUMode::Type mode)
{
	BenchSettings s = {};
	s.TestResolution = BBTestMode::HD;
	s.TestType = mode;
	Settings.push_back(s);
	s = BenchSettings();
	s.TestResolution = BBTestMode::QHD;
	s.TestType = mode;
	Settings.push_back(s);
	s = BenchSettings();
	s.TestResolution = BBTestMode::UHD;
	s.TestType = mode;
	Settings.push_back(s);
}

void LevelBenchMarker::Setup()
{
	 AddAllRes(MGPUMode::SFR);
	//AddAllRes(MGPUMode::SFR);
	for (int i = 0; i < MGPUMode::Limit - 1; i++)
	{
		//AddAllRes((MGPUMode::Type)(MGPUMode::SFR + i));
	}
	CloseOnFinish = true;
}

void LevelBenchMarker::Init()
{
	CameraObject->SetPosition(glm::vec3(0, 0, 0));
	Points.push_back(SplineNodes{ glm::vec3(0, 8, 16) ,glm::vec3(0,0,-1) });
	Points.push_back(SplineNodes{ glm::vec3(30, 10, 0) ,glm::vec3(1,0,0) });
	Points.push_back(SplineNodes{ glm::vec3(0, 12, -30) ,glm::vec3(0,0,1) });
	Points.push_back(SplineNodes{ glm::vec3(-30, 12, 0) ,glm::vec3(-1,0,0) });
	Points.push_back(Points[0]);

	CurrnetPointIndex = 1;
	Setup();//Name files
	RHI::GetRenderSettings()->SetRes(Settings[0].TestResolution);
	if (Engine::GetEPD()->Restart)
	{
		int index = Engine::GetEPD()->BenchIndex;
		CurrentSettingsIndex = index;
		TransitionToSetting(&Settings[index], index);
	}
	else
	{
		TransitionToSetting(&Settings[0], 0);
	}
}

void LevelBenchMarker::TransitionToSetting(BenchSettings* setting, int index)
{
	if (setting->TestType != RHI::GetMGPUSettings()->CurrnetTestMode)
	{
		Engine::GetEPD()->Restart = true;
		Engine::GetEPD()->MultiGPUMode = setting->TestType;
		Engine::GetEPD()->BenchIndex = index;
		Log::LogMessage("Rebooting to change MultiGPU Settings (Mode " + MGPUMode::ToString(setting->TestType) + ") ");
		Engine::RequestExit(Engine::RestartCode);
	}
	RHI::GetRenderSettings()->SetRes(setting->TestResolution);
	BaseWindow::StaticResize();
	Restart();
	CurrnetSetting = setting;
}

void LevelBenchMarker::End()
{
	PerfManager::EndBenchMark();
	CurrentSettingsIndex++;
	PreHeatTimer = PreHeatTime;
	if (CurrentSettingsIndex < Settings.size())
	{
		TransitionToSetting(&Settings[CurrentSettingsIndex], CurrentSettingsIndex);
	}
	else
	{
		Engine::GetEPD()->Restart = false;
		if (CloseOnFinish)
		{
			Engine::RequestExit(0);
		}
	}
}

void LevelBenchMarker::Update()
{
	if (CurrentSettingsIndex >= Settings.size())
	{
		return;
	}
	PreHeatTimer -= Engine::GetDeltaTime();
	if (PreHeatTimer > 0.0f)
	{
		return;
	}
	if (Once)
	{
		PerfManager::StartBenchMark(RenderSettings::ToString(CurrnetSetting->TestResolution) + "_" + MGPUMode::ToString(CurrnetSetting->TestType));
		Once = false;
	}
	if (Points.size() < 2 || CameraObject == nullptr)
	{
		return;
	}
	glm::vec3 CurrentPos = CameraObject->GetPosition();
	glm::vec3 NextPos = Points[CurrnetPointIndex].Pos;
	float distanceToNextPos = glm::distance2(NextPos, CurrentPos);
	if (CurrnetPointIndex == Points.size() - 1 && distanceToNextPos < CompletionDistance*CompletionDistance)
	{
		End();
		return;
	}
	if (distanceToNextPos < CompletionDistance*CompletionDistance)
	{
		PreHeatTimer = Points[CurrnetPointIndex].WaitTime;
		CurrnetPointIndex++;		
	}
	CurrnetPointIndex = glm::clamp(CurrnetPointIndex, 0, (int)Points.size() - 1);
	glm::vec3 dir = (NextPos - CurrentPos);
	if (glm::length2(dir) == 0)
	{
		return;
	}
	const glm::vec3 N_Dir = glm::normalize(dir);
	glm::vec3 NewPos = /*glm::mix(CurrentPos, NextPos, 0.01f);*/CurrentPos + N_Dir * 10 * Engine::GetDeltaTime();
	CameraObject->SetPosition(NewPos);
	Rot = Points[CurrnetPointIndex].Forward;
	glm::vec3 LastPoint = Points[CurrnetPointIndex - 1].Pos;
	float TotalDistnace = glm::length(Points[CurrnetPointIndex].Pos - LastPoint);
	float CurrentDist = glm::length(Points[CurrnetPointIndex].Pos - NewPos);
	float pc = CurrentDist / (TotalDistnace - 2.5f);
	if (glm::length2(Rot) == 0)
	{
		return;
	}
	pc = glm::clamp(pc, 0.0f, 1.0f);
	glm::quat qrot = glm::lookAtLH(CurrentPos, CurrentPos + Rot, glm::vec3(0, 1, 0));
	glm::quat qendrot = glm::lookAtLH(CurrentPos, CurrentPos + Points[CurrnetPointIndex - 1].Forward, glm::vec3(0, 1, 0));

	qrot = glm::slerp(qendrot, qrot, 1.0f - pc);
	CameraObject->SetRotation(qrot);
}

void LevelBenchMarker::Restart()
{
	CurrnetPointIndex = 0;
	CameraObject->SetPosition(Points[0].Pos);
	glm::vec3 CurrentPos = Points[0].Pos;
	glm::quat qrot = glm::lookAtLH(CurrentPos, CurrentPos + Points[0].Forward, glm::vec3(0, 1, 0));
	CameraObject->SetRotation(qrot);
	Once = true;
}
