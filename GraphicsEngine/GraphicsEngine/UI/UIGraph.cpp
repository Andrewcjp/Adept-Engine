#include "stdafx.h"
#include "UIGraph.h"
#include "LineDrawer.h"
#include "../Core/Performance/PerfManager.h"
UIGraph::UIGraph(LineDrawer* linebatch, int w, int h, int x, int y) :UIWidget(w, h, x, y)
{
	LineBatcher = linebatch;
	PointWidth = 10;
	GraphPoints = w / PointWidth * 2;
	CurrentFrameCounter = 10;
	FramesPerSample = 5;
}


UIGraph::~UIGraph()
{
}

void UIGraph::Render()
{
	LineBatcher->AddLine(glm::vec2(X, Y), glm::vec2(X, Y + mheight), glm::vec3(1, 1, 1));
	LineBatcher->AddLine(glm::vec2(X + mwidth, Y), glm::vec2(X + mwidth, Y + mheight), glm::vec3(1, 1, 1));
	LineBatcher->AddLine(glm::vec2(X, Y), glm::vec2(X + mwidth, Y), glm::vec3(1, 1, 1));
	//render points;
	RenderPoints(points, GraphColour);
	RenderPoints(Secondpoints, glm::vec3(1, 1, 0));
}
void UIGraph::RenderPoints(std::vector<float> &inpoints, glm::vec3 colour)
{
	int accum = 0;
	for (int i = 0; (i + 1) < inpoints.size(); i += 2)
	{
		LineBatcher->AddLine(glm::vec2(X + accum, Y + inpoints[i]), glm::vec2(X + accum + PointWidth, Y + inpoints[i + 1]), colour);
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
	if (CurrentFrameCounter > 0)
	{
		CurrentFrameCounter--;
		return;
	}
	CurrentFrameCounter = FramesPerSample;
	AddPoint(points, PerfManager::GetGPUTime()*2.5f);
	AddPoint(Secondpoints, PerfManager::GetCPUTime()*2.5f);
}
