#pragma once

struct TimerData;
class DynamicResolutionScaler
{
public:
	DynamicResolutionScaler();
	~DynamicResolutionScaler();
	void Init();
	void Tick();
	//todo: handle 4K at *4.0f
	float MaxResolutionScale = 4.0f;
	float MinResolutionScale = 0.5f;
	float UpdateRate = 0.2f;
	void SetTargetFrameRate(int rate);
private:
	float TargetGPUTimeMS = 16.0f;
	float MaxChange = 0.2f;
	float CurrentCooldown = 0.0f;
	float DeadZone = 0.5f;
	TimerData* Timer = nullptr;
};

