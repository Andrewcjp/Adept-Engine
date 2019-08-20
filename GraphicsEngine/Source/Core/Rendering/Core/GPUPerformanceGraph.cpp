#include "GPUPerformanceGraph.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
#include "DebugLineDrawer.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "RHI/RHI.h"
#include "RHI/RHITimeManager.h"
#include "RHI/DeviceContext.h"
#include <algorithm>
static ConsoleVariable IsActive("GPG", 0, ECVarType::ConsoleAndLaunch);

GPUPerformanceGraph::GPUPerformanceGraph()
{
	StartPos = glm::vec3(250, 150, 0);
	Scale = 100;
}

GPUPerformanceGraph::~GPUPerformanceGraph()
{}

void GPUPerformanceGraph::Render()
{
	if (!IsActive.GetBoolValue())
	{
		return;
	}
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		RenderGPU(i);
	}
}

void GPUPerformanceGraph::RenderGPU(int index)
{
	std::vector<GPUTimerPair*> data = RHI::GetDeviceContext(index)->GetTimeManager()->GetGPUTimers();
	if (data.size() == 0)
	{
		return;
	}
	std::vector<std::pair<float, GPUTimerPair*>> Timeline;
	for (int i = 0; i < data.size(); i++)
	{
		Timeline.push_back(std::make_pair(data[i]->Stamps[0], data[i]));
		Timeline.push_back(std::make_pair(data[i]->Stamps[1], data[i]));
	}
	struct
	{
		bool operator()(std::pair<float, GPUTimerPair*> a, std::pair<float, GPUTimerPair*> b) const
		{
			return a.first < b.first;
		}
	} customLess;
	std::sort(Timeline.begin(), Timeline.end(), customLess);
	int MaxStackSize = 0;
	std::vector<GPUTimerPair*> Stack;
	for (int i = 0; i < Timeline.size(); i++)
	{
		int Off = 0;
		if (Stack.size() == 0)
		{
			Stack.push_back(Timeline[i].second);
		}
		else
		{
			GPUTimerPair* Key = Timeline[i].second;
			bool found = false;
			for (int X = 0; X < Stack.size(); X++)
			{
				if (Stack[X] == Key)
				{
					Stack.erase(Stack.begin() + X);
					found = true;
					break;
				}
			}
			if (!found)
			{
				Stack.push_back(Timeline[i].second);
			}
			Off = Stack.size();
		}
		MaxStackSize = std::max(MaxStackSize, (int)Stack.size());
		Timeline[i].second->Offset = Off;
	}
	const glm::vec3 LocalPos = StartPos + glm::vec3(0, index * 30 * 3, 0);
	MainBarHeight = MaxStackSize * 20.0f;
	DrawBaseLine(PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Total GPU" + std::to_string(index))), index, LocalPos);
	for (int i = 0; i < data.size(); i++)
	{
		DrawLine(data[i], LocalPos);
	}
}

bool GPUPerformanceGraph::IsEnabled() const
{
	return IsActive.GetBoolValue();
}

void GPUPerformanceGraph::SetEnabled(bool state)
{
	IsActive.SetValue(state);
}

void GPUPerformanceGraph::DrawLine(GPUTimerPair * data, glm::vec3 LocalOffset)
{
	float CurrnetValue = data->Stamps[0] * maxV;
	float Endvalue = data->Stamps[1] * maxV;
	float Length = Endvalue - CurrnetValue;
	SetOffset(data->Owner, LocalOffset);
	LocalOffset.y -= data->Offset * 20.0f;
	const glm::vec3 Colour = glm::vec3(1);
	const glm::vec3 EndPos = LocalOffset + glm::vec3(Endvalue, 0, 0);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, 0, 0), EndPos, Colour);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, 0, 0), LocalOffset + glm::vec3(CurrnetValue, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(EndPos, EndPos + glm::vec3(0, EndLineHeight, 0), Colour);

	const float TextLength = data->Owner->name.length() * TextCharSize;
	if (Length > TextLength)
	{
		TextRenderer::instance->RenderText(data->Owner->name, LocalOffset.x + CurrnetValue + 4.0f, EndPos.y + 5.0f, TextScale, Colour);
	}
	else if (Length > TextCharSize * 2.0f)
	{
		int count = (int)std::ceil((TextLength - Length) / TextCharSize);
		std::string Final = data->Owner->name;
		Final.erase(Final.length() - count);
		const float newtextLength = Final.length() * TextCharSize;
		TextRenderer::instance->RenderText(Final, LocalOffset.x + CurrnetValue + 4.0f, EndPos.y + 5.0f, TextScale, Colour);
	}
}

void GPUPerformanceGraph::SetOffset(GPUTimer * data, glm::vec3 &LocalOffset)
{
	if (data->Type == ECommandListType::Copy)
	{
		LocalOffset.y -= MainBarHeight + 30.0f * 2;
	}
	else if (data->Type == ECommandListType::Compute || data->Type == ECommandListType::RayTracing)
	{
		LocalOffset.y -= MainBarHeight + 30.0f;
	}
}

void GPUPerformanceGraph::DrawBaseLine(TimerData* Timer, int GPUindex, glm::vec3 pos)
{
	float MaxValue = Timer->AVG->GetCurrentAverage()*Scale;
	maxV = MaxValue;
	const glm::vec3 Offsetpos = pos + glm::vec3(Timer->GPUStartOffset * Scale, 0.0f, 0.0f);
	const glm::vec3 Colour = glm::vec3(0);
	const glm::vec3 EndPos = Offsetpos + glm::vec3(MaxValue, 0, 0);
	TwoDrawer->AddLine(Offsetpos, EndPos, Colour);
	TwoDrawer->AddLine(Offsetpos + glm::vec3(0, EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(Offsetpos + glm::vec3(0, 0, 0), Offsetpos + glm::vec3(0, EndLineHeight, 0), Colour);
	TwoDrawer->AddLine(EndPos + glm::vec3(0, 0, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
	const float Offset = 70.0f* 2.5;
	TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Graphics", pos.x - Offset, EndPos.y + 5.0f, TextScale, glm::vec3(1));
	TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Compute", pos.x - Offset, EndPos.y + 5.0f - MainBarHeight - 30.0f, TextScale, glm::vec3(1));
	TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Copy", pos.x - Offset, EndPos.y + 5.0f - MainBarHeight - 30.0f * 2, TextScale, glm::vec3(1));
	TextRenderer::instance->RenderText(StringUtils::ToString(MaxValue / Scale) + "ms", EndPos.x + 10.0f, EndPos.y - 5.0f + 10.0f, TextScale, glm::vec3(1));
}
