#pragma once
#include "D3D12RHI.h"
#include "../Core/Utils/MovingAverage.h"
class D3D12TimeManager
{
public:
	static D3D12TimeManager* Instance;
	D3D12TimeManager();
	~D3D12TimeManager();
	static void Initialize(class DeviceContext* context);
	static void Destory();
	

	

	void UpdateTimers();
	void StartTimer(ID3D12GraphicsCommandList * ComandList);
	void StartTimer(RHICommandList * ComandList);
	void EndTimer(ID3D12GraphicsCommandList * ComandList);
	void EndTimer(RHICommandList * ComandList);

	UINT64 gpuTimeUS = 0;
	float gpuTimeMS = 0;
	float AVGgpuTimeMS = 0;

private:
	void Init(DeviceContext * context);
	MovingAverage avg = MovingAverage(50);
	ID3D12QueryHeap* m_timestampQueryHeaps;
	ID3D12Resource* m_timestampResultBuffers;
	UINT64 m_directCommandQueueTimestampFrequencies;
};

