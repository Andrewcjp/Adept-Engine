#pragma once
#include "Core/Utils/MovingAverage.h"
#include "RHI/RHITimeManager.h"
#include "RHI/RHITypes.h"
#define AVGTIME 50
#define ENABLE_GPUTIMERS 1
#if __has_include(<pix3.h>)
#define PIX_ENABLED 1
#else
#define PIX_ENABLED 0
#endif

class DeviceContext;
class D3D12DeviceContext;
class D3D12TimeManager : public RHITimeManager
{
public:

	D3D12TimeManager(DeviceContext* context);
	~D3D12TimeManager();
	void UpdateTimers() override;
	std::string GetTimerData() override;
	void SetTimerName(int index, std::string Name, ECommandListType::Type type = ECommandListType::Graphics) override;

	void StartTotalGPUTimer(RHICommandList * ComandList) override;
	void StartTimer(RHICommandList * ComandList, int index) override;
	void EndTimer(RHICommandList * ComandList, int index) override;
	void EndTotalGPUTimer(RHICommandList * ComandList) override;

	float GetTotalTime() override;
	void StartTimer(ID3D12GraphicsCommandList * ComandList, int index, bool IsCopy);
	void EndTimer(ID3D12GraphicsCommandList * ComandList, int index, bool IsCopy);

	void EndTotalGPUTimer(ID3D12GraphicsCommandList * ComandList);
	void ResolveTimeHeaps(RHICommandList * CommandList) override;
	void ResolveCopyTimeHeaps(RHICommandList * ComandList) override;
private:
	static UINT64 GPU0_TS;
	UINT64 StartTimeStamp = 0;
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
		UINT64 StartTS = 0;
		float StartOffset = 0.0f;
		MovingAverage StartOffsetavg = MovingAverage(AVGTIME);
	};
	int StatsGroupId = -1;
	bool TimerStarted = false;
#if GPUTIMERS_FULL
	const int MaxTimerCount = EGPUTIMERS::LIMIT;
	static const int TotalMaxTimerCount = EGPUTIMERS::LIMIT + EGPUCOPYTIMERS::LIMIT;
	const int MaxTimeStamps = MaxTimerCount * 2;
	const bool EnableCopyTimers = true;
#else 
	const int MaxTimerCount = 1;
	static const int TotalMaxTimerCount = 1;
	const int MaxTimeStamps = 2;
	const bool EnableCopyTimers = false;
#endif
	int MaxIndexInUse = 0;
	void Init(DeviceContext * context);
	void ProcessTimeStampHeaps(int count, ID3D12Resource * ResultBuffer, UINT64 ClockFreq, bool CopyList, int offset);

	MovingAverage avg = MovingAverage(AVGTIME);
	ID3D12QueryHeap* m_timestampQueryHeaps;
	ID3D12Resource* m_timestampResultBuffers;
	UINT64 m_directCommandQueueTimestampFrequencies = 1;
	UINT64 m_ComputeQueueFreqency = 1;
	UINT64 m_copyCommandQueueTimestampFrequencies = 1;
	GPUTimer TimeDeltas[TotalMaxTimerCount] = {};
	ID3D12QueryHeap* m_CopytimestampQueryHeaps = nullptr;
	ID3D12Resource* m_CopytimestampResultBuffers = nullptr;
	D3D12DeviceContext* Device = nullptr;
#if PIX_ENABLED
	std::wstring PixTimerNames[TotalMaxTimerCount] = {};
	LPCWSTR GetTimerNameForPIX(int index);
#endif
};

