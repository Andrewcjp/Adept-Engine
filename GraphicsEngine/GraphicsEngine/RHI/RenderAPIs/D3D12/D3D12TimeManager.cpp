#include "stdafx.h"
#include "D3D12TimeManager.h"
#include "../RHI/DeviceContext.h"
#include "D3D12CommandList.h"
#include <iomanip>
D3D12TimeManager::D3D12TimeManager(DeviceContext* context)
{
	Init(context);
}

D3D12TimeManager::~D3D12TimeManager()
{
#if ENABLE_GPUTIMERS
	m_timestampQueryHeaps->Release();
	m_timestampResultBuffers->Release();
#endif
}


void D3D12TimeManager::Init(DeviceContext* context)
{
#if ENABLE_GPUTIMERS
	// Create query heaps and result buffers.
	// Two timestamps for each frame.
	const UINT resultCount = MaxTimeStamps;//sync with count;
	const UINT resultBufferSize = resultCount * sizeof(UINT64);

	D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
	timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	timestampHeapDesc.Count = resultCount;

	ThrowIfFailed(context->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_timestampResultBuffers)));

	ThrowIfFailed(context->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_timestampQueryHeaps)));


	ThrowIfFailed(context->GetCommandQueue()->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies));
	int TimerItor = 0;
	for (int i = 0; i < MaxTimerCount; i++)
	{
		TimeDeltas[i].Startindex = TimerItor;
		TimeDeltas[i].Endindex = TimerItor + 1;
		TimerItor += 2;
	}
	SetTimerName(D3D12TimeManager::eGPUTIMERS::Total, "Total GPU");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::MainPass, "MainPass");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::PointShadows, "Point Shadow Time");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::DirShadows, "Dir Shadow Time");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::Text, "Text");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::UI, "UI");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::Skybox, "Skybox");
	SetTimerName(D3D12TimeManager::eGPUTIMERS::PostProcess, "PP");
#endif
}
void D3D12TimeManager::UpdateTimers()
{
#if ENABLE_GPUTIMERS
	D3D12_RANGE readRange = {};
	const D3D12_RANGE emptyRange = {};
	void* pData = nullptr;
	ThrowIfFailed(m_timestampResultBuffers->Map(0, &readRange, &pData));

	const UINT64* pTimestamps = reinterpret_cast<UINT64*>(static_cast<UINT8*>(pData) + readRange.Begin);
	for (int i = 0; i < MaxTimerCount; i++)
	{
		if (TimeDeltas[i].Used)
		{
			UINT64 delta = pTimestamps[TimeDeltas[i].Endindex] - pTimestamps[TimeDeltas[i].Startindex];
			float gpuTimeMS = (float)(delta * 1000) / (float)m_directCommandQueueTimestampFrequencies;
			TimeDeltas[i].avg.Add(gpuTimeMS);
		}
		else
		{
			//timer was never started 
			TimeDeltas[i].avg.Add(0);
		}
		TimeDeltas[i].Used = false;
	}
	// Unmap with an empty range (written range).
	m_timestampResultBuffers->Unmap(0, &emptyRange);

	AVGgpuTimeMS = TimeDeltas[0].avg.GetCurrentAverage();
#endif
}
std::string D3D12TimeManager::GetTimerData()
{
#if ENABLE_GPUTIMERS
	std::stringstream stream;
	stream.str("GPU: ");
	stream << std::fixed;
	float TrackedTime = 0.0f;
	for (int i = 0; i < MaxIndexInUse+1; i++)
	{
		stream << TimeDeltas[i].name << ": " << std::setprecision(3) << fabs(TimeDeltas[i].avg.GetCurrentAverage()) << " ";
		TrackedTime += fabs(TimeDeltas[i].avg.GetCurrentAverage());
	}
	stream << "Lost:" << TrackedTime - AVGgpuTimeMS;
	return stream.str();
#else
	return "GPU TIMERS DISABLED";
#endif
}

void D3D12TimeManager::SetTimerName(int index, std::string Name)
{
	if (index >= MaxTimerCount)
	{
		return;
	}
	ensure(index > -1);
	TimeDeltas[index].name = Name;
	MaxIndexInUse = std::max(index,MaxIndexInUse);
}


void D3D12TimeManager::StartTimer(RHICommandList* CommandList, int index)
{
#if ENABLE_GPUTIMERS
	if (index >= MaxTimerCount)
	{
		return;
	}
	ensure(index > -1);
	D3D12CommandList* List = (D3D12CommandList*)CommandList;
	StartTimer(List->GetCommandList(), index);
#endif
}

void D3D12TimeManager::EndTimer(RHICommandList* CommandList, int index)
{
#if ENABLE_GPUTIMERS
	if (index >= MaxTimerCount)
	{
		return;
	}
	ensure(index > -1);
	D3D12CommandList* List = (D3D12CommandList*)CommandList;
	EndTimer(List->GetCommandList(), index);
#endif
}

void D3D12TimeManager::StartTimer(ID3D12GraphicsCommandList * ComandList, int index)
{
#if ENABLE_GPUTIMERS
	ComandList->EndQuery(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Startindex);
	TimeDeltas[index].Used = true;
#endif
}

void D3D12TimeManager::EndTimer(ID3D12GraphicsCommandList* ComandList, int index)
{
#if ENABLE_GPUTIMERS
	ComandList->EndQuery(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Endindex);
#endif
}

void D3D12TimeManager::StartTotalGPUTimer(RHICommandList* ComandList)
{
#if ENABLE_GPUTIMERS
	if (TimerStarted)
	{
		return;
	}
	TimerStarted = true;
	StartTimer(ComandList, 0);
#endif
}

void D3D12TimeManager::EndTotalGPUTimer(RHICommandList* ComandList)
{
#if ENABLE_GPUTIMERS
	TimerStarted = false;
	EndTimer(ComandList, 0);
#endif
}

void D3D12TimeManager::EndTotalGPUTimer(ID3D12GraphicsCommandList* ComandList)
{
#if ENABLE_GPUTIMERS
	TimerStarted = false;
	EndTimer(ComandList, 0);
	ComandList->ResolveQueryData(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0, MaxTimerCount * 2, m_timestampResultBuffers, 0);
#endif
}