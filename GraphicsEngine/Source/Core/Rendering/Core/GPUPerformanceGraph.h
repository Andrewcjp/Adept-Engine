#pragma once
#include "Core\Performance\PerfManager.h"

class DebugLineDrawer;


///Class which draws lines to represent the GPU Pipeline(s)
class GPUPerformanceGraph
{
public:
	GPUPerformanceGraph();
	~GPUPerformanceGraph();
	void Render();
	void SetEnabled(bool state);
	void DrawLine(PerfManager::TimerData * data, float MaxValue, float& CurrnetValue, int index);
	void DrawBaseLine(float Time);
	glm::vec3 Pos = glm::vec3(0);
	float MaxLength = 100.0f;
	DebugLineDrawer* TwoDrawer = nullptr;
	float Scale = 10.0f;
private:

};

