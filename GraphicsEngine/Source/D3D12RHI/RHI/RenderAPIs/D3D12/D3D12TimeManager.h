#pragma once
#include "EngineGlobals.h"
#include "Core/Utils/MovingAverage.h"
#include "RHI/RHITypes.h"
#include "RHI/RHITimeManager.h"
#include <d3dx12.h>
#define AVGTIME 50
#define ENABLE_GPUTIMERS 1
#define MAX_TIMER_COUNT 8

class D3D12TimeManager : public RHITimeManager
{
public:
	
	D3D12TimeManager(class DeviceContext* context);
	~D3D12TimeManager();
	void UpdateTimers() override;
	std::string GetTimerData() override;
	void SetTimerName(int index, std::string Name) override;
	void StartTotalGPUTimer(RHICommandList * ComandList) override;
	void StartTimer(RHICommandList * ComandList, int index) override;
	void EndTimer(RHICommandList * ComandList, int index) override;
	void EndTotalGPUTimer(RHICommandList * ComandList) override;

	float GetTotalTime() override;
	void StartTimer(ID3D12GraphicsCommandList * ComandList, int index);
	void EndTimer(ID3D12GraphicsCommandList * ComandList, int index);
	void EndTotalGPUTimer(ID3D12GraphicsCommandList * ComandList);
private:
	float AVGgpuTimeMS = 0;	
	struct GPUTimer
	{
		std::string name;
		int Startindex = 0;
		int Endindex = 1;
		MovingAverage avg = MovingAverage(AVGTIME);
		bool Used = false;
		int Statid = -1;
		float RawTime = 0.0f;
	};
	int StatsGroupId = -1;
	bool TimerStarted = false;
#if GPUTIMERS_FULL
	const int MaxTimerCount = EGPUTIMERS::LIMIT;
	const int MaxTimeStamps = MaxTimerCount * 2;
#else 
	const int MaxTimerCount = 1;
	const int MaxTimeStamps =  2;
#endif
	int MaxIndexInUse = 0;
	void Init(DeviceContext * context);
	MovingAverage avg = MovingAverage(AVGTIME);
	ID3D12QueryHeap* m_timestampQueryHeaps;
	ID3D12Resource* m_timestampResultBuffers;
	UINT64 m_directCommandQueueTimestampFrequencies;
	GPUTimer TimeDeltas[EGPUTIMERS::LIMIT];
};

