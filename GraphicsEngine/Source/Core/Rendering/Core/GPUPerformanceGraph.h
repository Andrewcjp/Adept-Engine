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

	void RenderGPU(int index);

	void SetEnabled(bool state);
	void DrawLine(PerfManager::TimerData * data, glm::vec3 pos, float& CurrnetValue, int index);
	void DrawBaseLine(float Time, int GPUindex, glm::vec3 pos);
	glm::vec3 StartPos = glm::vec3(0);
	float MaxLength = 100.0f;
	DebugLineDrawer* TwoDrawer = nullptr;
	float Scale = 10.0f;
private:
	float EndLineHeight = 20;
};

