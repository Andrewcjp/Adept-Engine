#include "Source/BleedOutGame/BleedOutPCH.h"
#include "LevelBenchMarker.h"
#include "Core/Performance/PerfManager.h"
#include "Core/BaseWindow.h"


LevelBenchMarker::LevelBenchMarker()
{}

LevelBenchMarker::~LevelBenchMarker()
{}

void LevelBenchMarker::Setup()
{
	BenchSettings s = {};
	s.TestResolution = BBTestMode::HD;
	Settings.push_back(s);
	s = BenchSettings();
	s.TestResolution = BBTestMode::QHD;
	Settings.push_back(s);
	s = BenchSettings();
	s.TestResolution = BBTestMode::UHD;
	Settings.push_back(s);
}

void LevelBenchMarker::Init()
{
	CameraObject->SetPosition(glm::vec3(0, 0, 0));
	Points.push_back(SplineNodes{ glm::vec3(0, 0, 0) ,glm::vec3(0,0,1) });
	Points.push_back(SplineNodes{ glm::vec3(0, 10, 5),glm::vec3(0,0,-1) });
	Points.push_back(SplineNodes{ glm::vec3(0, 10, 30) ,glm::vec3(0,0,1) });
	CurrnetPointIndex = 1;
	Setup();//Name files
	RHI::GetRenderSettings()->SetRes(Settings[0].TestResolution);
}

void LevelBenchMarker::End()
{
	PerfManager::EndBenchMark();
	CurrentSettingsIndex++;
	PreHeat = PreHeatFrames * 2;
	if (CurrentSettingsIndex < Settings.size())
	{
		RHI::GetRenderSettings()->SetRes(Settings[CurrentSettingsIndex].TestResolution);
		BaseWindow::StaticResize();
		Restart();
	}
}

void LevelBenchMarker::Update()
{
	if (CurrentSettingsIndex >= Settings.size())
	{
		return;
	}
	PreHeat--;
	if (PreHeat > 0)
	{
		return;
	}
	if (PreHeat == 0)
	{
		PerfManager::StartBenchMark(RenderSettings::ToString(Settings[CurrentSettingsIndex].TestResolution));
		PreHeat = -1;
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
}
