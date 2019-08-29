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
static ConsoleVariable IsActive("GPG.enabled", 0, ECVarType::ConsoleAndLaunch);

GPUPerformanceGraph::GPUPerformanceGraph()
{
	StartPos = glm::vec3(250, 500, 0);
	Scale = 50;
	SimpleModeOnly = false;
}

GPUPerformanceGraph::~GPUPerformanceGraph()
{}

void GPUPerformanceGraph::Render()
{
	if (!IsActive.GetBoolValue())
	{
		return;
	}
	LastBarOffset = 0;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		RenderGPU(i, ECommandListType::Graphics);
		RenderGPU(i, ECommandListType::Compute);
#if SEPERATE_RAYTRACING_TIMERS
		RenderGPU(i, ECommandListType::RayTracing);
#endif
		RenderGPU(i, ECommandListType::Copy);
	}
}

void GPUPerformanceGraph::RenderGPU(int index, ECommandListType::Type Listtype)
{
	std::vector<GPUTimerPair*> data = RHI::GetDeviceContext(index)->GetTimeManager()->GetGPUTimers(Listtype);
	if (data.size() == 0)
	{
		MainBarHeight = 0.0f;
		const glm::vec3 LocalPos = StartPos + glm::vec3(0, -index * BarSpacing * 3, 0);
		DrawBaseLine(PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Total GPU" + std::to_string(index))), index, LocalPos, Listtype);
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
	const glm::vec3 LocalPos = StartPos + glm::vec3(0, -index * BarSpacing * 3, 0);
	MainBarHeight = MaxStackSize * StackBarOffset;
	DrawBaseLine(PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Total GPU" + std::to_string(index))), index, LocalPos, Listtype);
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
	if (SimpleModeOnly && data->Offset != 0)
	{
		return;
	}
	float CurrnetValue = data->Stamps[0] * maxV;
	float Endvalue = data->Stamps[1] * maxV;
	float Length = Endvalue - CurrnetValue;
	LocalOffset.y -= BarOffsets[data->Owner->Type];
	LocalOffset.y -= data->Offset * StackBarOffset;
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
	else if (Length > TextCharSize * (EnableFlags ? 5.0f : 2.0f))
	{
		int count = (int)std::ceil((TextLength - Length) / TextCharSize);
		std::string Final = data->Owner->name;
		Final.erase(Final.length() - count);
		const float newtextLength = Final.length() * TextCharSize;
		TextRenderer::instance->RenderText(Final, LocalOffset.x + CurrnetValue + 4.0f, EndPos.y + 5.0f, TextScale, Colour);
	}
	else if (data->Offset == 0 && EnableFlags)
	{
		TextRenderer::instance->RenderText(data->Owner->name, LocalOffset.x + CurrnetValue + 4.0f, EndPos.y + 5.0f + FlagTextOffset, TextScale, Colour);
		TwoDrawer->AddLine(LocalOffset + glm::vec3(CurrnetValue, 0, 0), LocalOffset + glm::vec3(CurrnetValue, 5.0f + FlagTextOffset, 0), Colour);
	}
}



void GPUPerformanceGraph::DrawBaseLine(TimerData * Timer, int GPUindex, glm::vec3 pos, ECommandListType::Type ListType)
{
	if (Timer == nullptr)
	{
		return;
	}
	float MaxValue = Timer->AVG->GetCurrentAverage()*Scale;
	maxV = MaxValue;
	glm::vec3 Offsetpos = pos + glm::vec3(Timer->GPUStartOffset * Scale, 0.0f, 0.0f);
	const glm::vec3 Colour = glm::vec3(0);
	glm::vec3 EndPos = Offsetpos + glm::vec3(MaxValue, 0, 0);
	const float Offset = 70.0f* 2.5;

	if (ListType == ECommandListType::Graphics)
	{
		Offsetpos.y -= LastBarOffset;
		EndPos.y -= LastBarOffset;
		TwoDrawer->AddLine(Offsetpos, EndPos, Colour);
		TwoDrawer->AddLine(Offsetpos + glm::vec3(0, EndLineHeight, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
		TwoDrawer->AddLine(Offsetpos + glm::vec3(0, 0, 0), Offsetpos + glm::vec3(0, EndLineHeight, 0), Colour);
		TwoDrawer->AddLine(EndPos + glm::vec3(0, 0, 0), EndPos + glm::vec3(0, EndLineHeight, 0), Colour);
		BarOffsets[ECommandListType::Graphics] = 0.0f;
		TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Graphics", pos.x - Offset, EndPos.y + 5.0f, TextScale, glm::vec3(1));
		TextRenderer::instance->RenderText(StringUtils::ToString(MaxValue / Scale) + "ms", EndPos.x + 10.0f, EndPos.y - 5.0f + 10.0f, TextScale, glm::vec3(1));
	}
	else if (ListType == ECommandListType::Compute)
	{
		LastBarOffset += MainBarHeight + BarSpacing;
		BarOffsets[ECommandListType::Compute] = LastBarOffset;
		TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Compute", pos.x - Offset, EndPos.y + 5.0f - LastBarOffset, TextScale, glm::vec3(1));
	}
#if SEPERATE_RAYTRACING_TIMERS
	else if (ListType == ECommandListType::RayTracing)
	{
		LastBarOffset += MainBarHeight + BarSpacing;
		BarOffsets[ECommandListType::RayTracing] = LastBarOffset;
		TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " RayTracing", pos.x - Offset, EndPos.y + 5.0f - LastBarOffset, TextScale, glm::vec3(1));
	}
#endif
	else if (ListType == ECommandListType::Copy)
	{
		LastBarOffset += MainBarHeight + BarSpacing;
		BarOffsets[ECommandListType::Copy] = LastBarOffset;
		TextRenderer::instance->RenderText("GPU_" + std::to_string(GPUindex) + " Copy", pos.x - Offset, EndPos.y + 5.0f - LastBarOffset, TextScale, glm::vec3(1));
	}

}
