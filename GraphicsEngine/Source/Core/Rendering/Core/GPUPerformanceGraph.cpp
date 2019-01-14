#include "GPUPerformanceGraph.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "DebugLineDrawer.h"
#include "../Renderers/TextRenderer.h"
static ConsoleVariable IsActive("GPG", 1, ECVarType::ConsoleAndLaunch);

GPUPerformanceGraph::GPUPerformanceGraph()
{
	Pos = glm::vec3(100, 100, 0);
	Scale = 20;
}

GPUPerformanceGraph::~GPUPerformanceGraph()
{}

void GPUPerformanceGraph::Render()
{
	if (!IsActive.GetBoolValue())
	{
		return;
	}
	std::vector<PerfManager::TimerData*> data = PerfManager::Get()->GetAllGPUTimers("GPU_0");
	if (data.size() == 0)
	{
		return;
	}
	float MaxValue = data[0]->AVG->GetCurrentAverage()*Scale;
	DrawBaseLine(MaxValue);
	float CurrentValue = 0.0f;
	for (int i = 1; i < data.size(); i++)
	{
		DrawLine(data[i], MaxValue, CurrentValue, i);
	}
}

void GPUPerformanceGraph::SetEnabled(bool state)
{
	IsActive.SetValue(state);
}

const float EndLineHeight = 10;
void GPUPerformanceGraph::DrawLine(PerfManager::TimerData * data, float MaxValue, float& CurrnetValue, int index)
{
	float Length = (data->AVG->GetCurrentAverage() * Scale);
	if (Length < 1.0f)
	{
		CurrnetValue += Length;
		return;
	}
	const glm::vec3 LocalOffset = glm::vec3(0, /*index * 20*/0, 0) + Pos;
	const glm::vec3 Colour = glm::vec3(1);
	const glm::vec3 EndPos = LocalOffset + glm::vec3(CurrnetValue + Length, 0, 0);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, 0, 0), EndPos, Colour);

	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, -EndLineHeight, 0), LocalOffset + glm::vec3(CurrnetValue, EndLineHeight, 0), Colour);

	TwoDrawer->AddLine(EndPos + glm::vec3(0, -EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);

	TextRenderer::instance->RenderText(data->name, EndPos.x, EndPos.y + 20 + (index * 20), 0.4, Colour);
	CurrnetValue += Length;
}

void GPUPerformanceGraph::DrawBaseLine(float Time)
{
	float Length = Time;
	const glm::vec3 Colour = glm::vec3(0);
	const glm::vec3 EndPos = Pos + glm::vec3(Length, 0, 0);
	TwoDrawer->AddLine(Pos, EndPos, Colour);
	TwoDrawer->AddLine(Pos + glm::vec3(0, -EndLineHeight, 0), Pos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(EndPos + glm::vec3(0, -EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	TextRenderer::instance->RenderText("GPU_0 ", Pos.x - 70, EndPos.y - 5, 0.4, glm::vec3(1));
	TextRenderer::instance->RenderText(std::to_string(Time / Scale) + "ms", EndPos.x + 10, EndPos.y - 5, 0.4, glm::vec3(1));
}
