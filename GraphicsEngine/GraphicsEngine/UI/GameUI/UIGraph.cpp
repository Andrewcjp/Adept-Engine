#include "stdafx.h"
#include "UIGraph.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Performance/PerfManager.h"
UIGraph::UIGraph(DebugLineDrawer* linebatch, int w, int h, int x, int y) :UIWidget(w, h, x, y)
{
	LineBatcher = linebatch;
	PointWidth = 10;
	GraphPoints = w / PointWidth * 2;
	CurrentFrameCounter = 10;
	FramesPerSample = 1;
}

UIGraph::~UIGraph()
{
}

void UIGraph::Render()
{
	LineBatcher->AddLine(glm::vec3(X, Y,0), glm::vec3(X, Y + mheight, 0), glm::vec3(1, 1, 1));
	LineBatcher->AddLine(glm::vec3(X + mwidth, Y, 0), glm::vec3(X + mwidth, Y + mheight, 0), glm::vec3(1, 1, 1));
	LineBatcher->AddLine(glm::vec3(X, Y,0), glm::vec3(X + mwidth, Y, 0), glm::vec3(1, 1, 1));
	//render points;
	RenderPoints(GPUpoints, glm::vec3(1, 1, 0));
	RenderPoints(CPUpoints, glm::vec3(1, 0, 0));
	RenderPoints(FrameTimepoints, glm::vec3(0, 1, 0));
}

void UIGraph::RenderPoints(std::vector<float> &inpoints, glm::vec3 colour)
{
	int accum = 0;
	for (int i = 0; (i + 1) < inpoints.size(); i += 2)
	{
		LineBatcher->AddLine(glm::vec3(X + accum, Y + inpoints[i], 0), glm::vec3(X + accum + PointWidth, Y + inpoints[i + 1], 0), colour);
		accum += PointWidth;
	}
}

void UIGraph::AddPoint(std::vector<float> &inpoints, float value)
{
	if (inpoints.size() > GraphPoints)
	{
		inpoints.pop_back();
	}
	inpoints.emplace(inpoints.begin(), value);
}

void UIGraph::UpdateData()
{
	CurrentFrameCounter--;
	if (CurrentFrameCounter > 0)
	{		
		return;
	}
	CurrentFrameCounter = FramesPerSample;
	const float Scale = 2.5f;
	AddPoint(GPUpoints, PerfManager::GetGPUTime()*Scale);
	AddPoint(CPUpoints, PerfManager::GetCPUTime()*Scale);
	AddPoint(FrameTimepoints, PerfManager::GetDeltaTime()*1000.0f*Scale);
}
