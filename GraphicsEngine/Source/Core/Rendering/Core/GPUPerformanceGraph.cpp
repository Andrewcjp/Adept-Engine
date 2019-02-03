#include "GPUPerformanceGraph.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
#include "DebugLineDrawer.h"
#include "Rendering/Renderers/TextRenderer.h"
static ConsoleVariable IsActive("GPG", 0, ECVarType::ConsoleAndLaunch);

GPUPerformanceGraph::GPUPerformanceGraph()
{
	StartPos = glm::vec3(250, 150, 0);
	Scale = 50;
}

GPUPerformanceGraph::~GPUPerformanceGraph()
{}

void GPUPerformanceGraph::Render()
{
	if (!IsActive.GetBoolValue())
	{
		return;
	}
	RenderGPU(0);
	RenderGPU(1);
	return;
}

void GPUPerformanceGraph::RenderGPU(int index)
{
	std::vector<PerfManager::TimerData*> data = PerfManager::Get()->GetAllGPUTimers("GPU_" + std::to_string(index));
	if (data.size() == 0)
	{
		return;
	}
	const glm::vec3 LocalPos = StartPos + glm::vec3(0, index * 30 * 3, 0);
	float MaxValue = data[0]->AVG->GetCurrentAverage()*Scale;
	DrawBaseLine(MaxValue, index, LocalPos);
	float CurrentValue = 0.0f;
	for (int i = 1; i < data.size(); i++)
	{
		DrawLine(data[i], LocalPos, CurrentValue, i);
	}
}

void GPUPerformanceGraph::SetEnabled(bool state)
{
	IsActive.SetValue(state);
}

void GPUPerformanceGraph::DrawLine(PerfManager::TimerData * data, glm::vec3 LocalOffset, float& CurrnetValue, int index)
{
	CurrnetValue = data->GPUStartOffset*Scale;
	float Length = (data->AVG->GetCurrentAverage() * Scale);
	if (Length < 1.0f)
	{
		CurrnetValue += Length;
		return;
	}
	if (data->TimerType == ECommandListType::Copy)
	{
		LocalOffset.y -= 30.0f * 2;
	}
	else if (data->TimerType == ECommandListType::Compute)
	{
		LocalOffset.y -= 30.0f;
	}
	const glm::vec3 Colour = glm::vec3(1);
	const glm::vec3 EndPos = LocalOffset + glm::vec3(CurrnetValue + Length, 0, 0);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, 0, 0), EndPos, Colour);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, 0, 0), LocalOffset + glm::vec3(CurrnetValue, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(EndPos, EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	const float TextCharSize = 12.0f;
	const float TextLength = data->name.length() * TextCharSize;
	if (Length > TextLength)
	{
		TextRenderer::instance->RenderText(data->name, LocalOffset.x + CurrnetValue + 4.0f, EndPos.y + 5.0f, 0.4f, Colour);
	}
	else if (Length > TextCharSize * 2.0f)
	{
		int count = (int)std::ceil((TextLength - Length) / TextCharSize);
		std::string Final = data->name;
		Final.erase(Final.length() - count);
		const float newtextLength = Final.length() * TextCharSize;
		TextRenderer::instance->RenderText(Final, LocalOffset.x + CurrnetValue + 4.0f, EndPos.y + 5.0f, 0.4f, Colour);
	}
	CurrnetValue += Length;
}

void GPUPerformanceGraph::DrawBaseLine(float Time, int GPUindex, glm::vec3 pos)
{
	float Length = Time;
	const glm::vec3 Colour = glm::vec3(0);
	const glm::vec3 EndPos = pos + glm::vec3(Length, 0, 0);
	TwoDrawer->AddLine(pos, EndPos, Colour);
	TwoDrawer->AddLine(pos + glm::vec3(0, EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(pos + glm::vec3(0, 0, 0), pos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(EndPos + glm::vec3(0, 0, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	const float Offset = 70.0f* 2.5;
	TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Graphics", pos.x - Offset, EndPos.y + 5.0f, 0.4f, glm::vec3(1));
	TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Compute", pos.x - Offset, EndPos.y + 5.0f - 30.0f, 0.4f, glm::vec3(1));
	TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Copy", pos.x - Offset, EndPos.y + 5.0f - 30.0f * 2, 0.4f, glm::vec3(1));
	TextRenderer::instance->RenderText(StringUtils::ToStringFloat(Time / Scale) + "ms", EndPos.x + 10.0f, EndPos.y - 5.0f + 10.0f, 0.4f, glm::vec3(1));
}
