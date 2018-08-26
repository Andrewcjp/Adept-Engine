#include "stdafx.h"
#include "D3D12TimeManager.h"
#include "D3D12CommandList.h"
#include <iomanip>
#include "Core/Performance/PerfManager.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"

D3D12TimeManager::D3D12TimeManager(DeviceContext* context) :RHITimeManager(context)
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
	D3D12DeviceContext* dcontext = (D3D12DeviceContext*)context;
#if ENABLE_GPUTIMERS
	// Create query heaps and result buffers.
	// Two timestamps for each frame.
	const UINT resultCount = MaxTimeStamps;//sync with count;
	const UINT resultBufferSize = resultCount * sizeof(UINT64);

	D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
	timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	timestampHeapDesc.Count = resultCount;

	ThrowIfFailed(dcontext->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_timestampResultBuffers)));

	ThrowIfFailed(dcontext->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_timestampQueryHeaps)));

	if (dcontext->GetCaps().SupportsCopyTimeStamps)
	{
		timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP;

		ThrowIfFailed(dcontext->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_CopytimestampResultBuffers)));

		ThrowIfFailed(dcontext->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_CopytimestampQueryHeaps)));
	}

	ThrowIfFailed(dcontext->GetCommandQueue()->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies));
	ThrowIfFailed(dcontext->GetCommandQueueFromEnum(DeviceContextQueue::Copy)->GetTimestampFrequency(&m_copyCommandQueueTimestampFrequencies));
	

	int TimerItor = 0;
	for (int i = 0; i < MaxTimerCount; i++)
	{
		TimeDeltas[i].Startindex = TimerItor;
		TimeDeltas[i].Endindex = TimerItor + 1;
		TimerItor += 2;
	}

	std::string GPUName = "GPU_";
	GPUName.append(std::to_string(context->GetDeviceIndex()));
	StatsGroupId = PerfManager::Get()->GetGroupId(GPUName.c_str());

	SetTimerName(EGPUTIMERS::Total, "Total GPU");
	SetTimerName(EGPUTIMERS::MainPass, "Main Pass");
	SetTimerName(EGPUTIMERS::DeferredWrite, "Deferred Write");
	SetTimerName(EGPUTIMERS::DeferredLighting, "Deferred Lighting");
	SetTimerName(EGPUTIMERS::Skybox, "Skybox");
	SetTimerName(EGPUTIMERS::PointShadows, "Point Shadow");
	SetTimerName(EGPUTIMERS::DirShadows, "Dir Shadow");
	SetTimerName(EGPUTIMERS::Text, "Text");
	SetTimerName(EGPUTIMERS::UI, "UI Draw");
	SetTimerName(EGPUTIMERS::PostProcess, "Post Processing");
	SetTimerName(EGPUTIMERS::ShadowPreSample, "Shadow PreSample");

	SetTimerName(EGPUTIMERS::MGPUCopy, "MGPU Copy");
	CopyListTimers[EGPUTIMERS::MGPUCopy] = true;
#endif
}

void D3D12TimeManager::ProcessTimeStampHeaps(int count, ID3D12Resource* ResultBuffer, float ClockFreq,bool IsCopyList)
{
	D3D12_RANGE readRange = {};
	const D3D12_RANGE emptyRange = {};
	void* pData = nullptr;
	ThrowIfFailed(ResultBuffer->Map(0, &readRange, &pData));

	const UINT64* pTimestamps = reinterpret_cast<UINT64*>(static_cast<UINT8*>(pData) + readRange.Begin);
	for (int i = 0; i < count; i++)
	{
		if (CopyListTimers[i] != IsCopyList)
		{
			continue;
		}
		if (TimeDeltas[i].Used)
		{
			UINT64 delta = pTimestamps[TimeDeltas[i].Endindex] - pTimestamps[TimeDeltas[i].Startindex];
			float gpuTimeMS = (float)(delta * 1000) / (float)ClockFreq;
			TimeDeltas[i].RawTime = gpuTimeMS;
			TimeDeltas[i].avg.Add(gpuTimeMS);
		}
		else
		{
			//timer was never started 
			TimeDeltas[i].avg.Add(0);
		}
	}
	// Unmap with an empty range (written range).
	ResultBuffer->Unmap(0, &emptyRange);
	
}

void D3D12TimeManager::UpdateTimers()
{
#if ENABLE_GPUTIMERS
	if (m_CopytimestampResultBuffers != nullptr)
	{
		ProcessTimeStampHeaps(MaxTimerCount, m_CopytimestampResultBuffers, m_copyCommandQueueTimestampFrequencies,true);
	}
	ProcessTimeStampHeaps(MaxTimerCount, m_timestampResultBuffers, m_directCommandQueueTimestampFrequencies,false);

	AVGgpuTimeMS = TimeDeltas[0].avg.GetCurrentAverage();
	for (int i = 0; i < MaxTimerCount; i++)
	{
		if (!TimeDeltas[i].Used)
		{
			continue;
		}
		PerfManager::Get()->UpdateGPUStat(TimeDeltas[i].Statid, TimeDeltas[i].RawTime);
	}
	for (int i = 0; i < MaxTimerCount; i++)
	{
		TimeDeltas[i].Used = false;
	}
#endif
}

std::string D3D12TimeManager::GetTimerData()
{
#if ENABLE_GPUTIMERS
	std::stringstream stream;
	stream.str("GPU: ");
	stream << std::fixed;
	float TrackedTime = 0.0f;
	for (int i = 0; i < MaxIndexInUse + 1; i++)
	{
		stream << TimeDeltas[i].name << ": " << std::setprecision(3) << fabs(TimeDeltas[i].avg.GetCurrentAverage()) << " ";
		if (i != 0)
		{
			TrackedTime += fabs(TimeDeltas[i].avg.GetCurrentAverage());
		}
	}
	stream << "Lost:" << AVGgpuTimeMS - TrackedTime;
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
	MaxIndexInUse = std::max(index, MaxIndexInUse);

	TimeDeltas[index].Statid = PerfManager::Get()->GetTimerIDByName(Name + std::to_string(Context->GetDeviceIndex()));
	PerfManager::Get()->AddTimer(TimeDeltas[index].Statid, StatsGroupId);
	if (PerfManager::Get()->GetTimerData(TimeDeltas[index].Statid) != nullptr)
	{
		PerfManager::Get()->GetTimerData(TimeDeltas[index].Statid)->name = Name;
	}

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
	StartTimer(List->GetCommandList(), index, List->IsCopyList());
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
	EndTimer(List->GetCommandList(), index, List->IsCopyList());
#endif
}

float D3D12TimeManager::GetTotalTime()
{
	return AVGgpuTimeMS;
}

void D3D12TimeManager::StartTimer(ID3D12GraphicsCommandList * ComandList, int index, bool IsCopy)
{
#if ENABLE_GPUTIMERS
	if (IsCopy)
	{

		ComandList->EndQuery(m_CopytimestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Startindex);
	}
	else
	{
		ComandList->EndQuery(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Startindex);
	}
	TimeDeltas[index].Used = true;
#endif
}

void D3D12TimeManager::EndTimer(ID3D12GraphicsCommandList* ComandList, int index, bool IsCopy)
{
#if ENABLE_GPUTIMERS
	if (IsCopy)
	{
		ComandList->EndQuery(m_CopytimestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Endindex);
	}
	else
	{
		ComandList->EndQuery(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Endindex);
	}
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
	EndTotalGPUTimer(((D3D12CommandList*)ComandList)->GetCommandList());
#endif
}

void D3D12TimeManager::EndTotalGPUTimer(ID3D12GraphicsCommandList* ComandList)
{
#if ENABLE_GPUTIMERS
	TimerStarted = false;
	EndTimer(ComandList, 0, false);
	ComandList->ResolveQueryData(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0, MaxTimerCount * 2, m_timestampResultBuffers, 0);
#endif
}

void D3D12TimeManager::ResolveCopyTimeHeaps(RHICommandList* CommandList)
{
	ensure(CommandList->IsCopyList());
	D3D12CommandList* List = (D3D12CommandList*)CommandList;
	List->GetCommandList()->ResolveQueryData(m_CopytimestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0, MaxTimerCount * 2, m_CopytimestampResultBuffers, 0);
}