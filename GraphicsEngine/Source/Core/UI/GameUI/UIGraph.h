#pragma once
#include "UI/Core/UIWidget.h"

class DebugLineDrawer;
class UIGraph : public UIWidget
{
public:
	UIGraph(DebugLineDrawer* linebatch, int w, int h, int x = 0, int y = 0);
	~UIGraph();
	void Render();
	void RenderPoints(std::vector<float>& inpoints, glm::vec3 colour);
	void AddPoint(std::vector<float>& inpoints, float value);
	void UpdateData() override;
private:
	DebugLineDrawer* LineBatcher;
	std::vector<float> GPUpoints;
	std::vector<float> CPUpoints;
	std::vector<float> FrameTimepoints;
	glm::vec3 GraphColour = glm::vec3(0, 1, 0);
	int GraphPoints = 50;
	int PointWidth = 3;
	int test = 0;
	int FramesPerSample = 1;
	int CurrentFrameCounter = 0;
};

