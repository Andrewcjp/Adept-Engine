#pragma once
#include "UIWidget.h"
#include <vector>
class LineDrawer;
class UIGraph : public UIWidget
{
public:
	UIGraph(LineDrawer* linebatch, int w, int h, int x = 0, int y = 0);
	~UIGraph();
	void Render();
	void RenderPoints(std::vector<float>& inpoints, glm::vec3 colour);
	void AddPoint(std::vector<float>& inpoints, float value);
	void UpdateData() override;
private:
	LineDrawer* LineBatcher;
	std::vector<float> points;
	std::vector<float> Secondpoints;
	glm::vec3 GraphColour = glm::vec3(0, 1, 0);
	int GraphPoints = 50;
	int PointWidth = 3;
	int test = 0;
	int FramesPerSample = 2;
	int CurrentFrameCounter = 0;
};

