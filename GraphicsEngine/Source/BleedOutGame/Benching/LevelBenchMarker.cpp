#include "Source/BleedOutGame/BleedOutPCH.h"
#include "LevelBenchMarker.h"
#include "Core/Performance/PerfManager.h"


LevelBenchMarker::LevelBenchMarker()
{}

LevelBenchMarker::~LevelBenchMarker()
{}

void LevelBenchMarker::Init()
{
	CameraObject->SetPosition(glm::vec3(0, 0, 0));
	Points.push_back(SplineNodes{ glm::vec3(0, 0, 0) });
	Points.push_back(SplineNodes{ glm::vec3(0, 10, 5) });
	Points.push_back(SplineNodes{ glm::vec3(0, 10, 30) });
	CurrnetPointIndex = 1;
	
}

void LevelBenchMarker::End()
{
	PerfManager::EndBenchMark();
}

void LevelBenchMarker::Update()
{
	PreHeat--;
	if (PreHeat > 0)
	{
		return;
	}
	if (PreHeat == 0)
	{
		PerfManager::StartBenchMark();
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


}
