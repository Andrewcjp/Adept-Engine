#include "D3D12TimeManager.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h" 
#if PIX_ENABLED
#define PROFILE_BUILD 
#include <pix3.h>
#endif

D3D12TimeManager::D3D12TimeManager(DeviceContext* context) :RHITimeManager(context)
{
	Init(context);
}

D3D12TimeManager::~D3D12TimeManager()
{
#if ENABLE_GPUTIMERS
	SafeRelease(m_timestampQueryHeaps);
	SafeRelease(m_timestampResultBuffers);
	SafeRelease(m_CopytimestampResultBuffers);
	SafeRelease(m_CopytimestampQueryHeaps);
#endif
}

void D3D12TimeManager::Init(DeviceContext* context)
{
	Device = (D3D12DeviceContext*)context;
#if ENABLE_GPUTIMERS
	// Create query heaps and result buffers.
	// Two timestamps for each frame.
	const UINT resultCount = MaxTimeStamps;//sync with count;
	const UINT resultBufferSize = resultCount * sizeof(UINT64);

	D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
	timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	timestampHeapDesc.Count = resultCount;

	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_timestampResultBuffers)));

	ThrowIfFailed(Device->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_timestampQueryHeaps)));
#if GPUTIMERS_FULL
	if (Device->GetCaps().SupportsCopyTimeStamps)
	{
		timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP;
		UINT CopyResultBuffesize = EGPUCOPYTIMERS::LIMIT * 2 * sizeof(UINT64);
		ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(CopyResultBuffesize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_CopytimestampResultBuffers)));

		ThrowIfFailed(Device->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_CopytimestampQueryHeaps)));
		ThrowIfFailed(Device->GetCommandQueueFromEnum(DeviceContextQueue::Copy)->GetTimestampFrequency(&m_copyCommandQueueTimestampFrequencies));
	}
#endif
	ThrowIfFailed(Device->GetCommandQueue()->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies));
	ThrowIfFailed(Device->GetCommandQueueFromEnum(DeviceContextQueue::Compute)->GetTimestampFrequency(&m_ComputeQueueFreqency));
	ensure(m_ComputeQueueFreqency == m_directCommandQueueTimestampFrequencies);
	int TimerItor = 0;
	for (int i = 0; i < TotalMaxTimerCount; i++)
	{
		if (i == CopyOffset)
		{
			TimerItor = 0;
		}
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
	SetTimerName(EGPUTIMERS::ParticleDraw, "Particle Draw");
	SetTimerName(EGPUTIMERS::ParticleSimulation, "Particle Sim", ECommandListType::Compute);
	SetTimerName(EGPUTIMERS::GPU0WaitOnGPU1, "GPU0 Wait On GPU1");
	SetTimerName(CopyOffset + EGPUCOPYTIMERS::MGPUCopy, "MGPU Copy", ECommandListType::Copy);
	SetTimerName(CopyOffset + EGPUCOPYTIMERS::SFRMerge, "SFR Merge", ECommandListType::Copy);
	SetTimerName(CopyOffset + EGPUCOPYTIMERS::ShadowCopy, "Shadow Copy", ECommandListType::Copy);
	SetTimerName(CopyOffset + EGPUCOPYTIMERS::ShadowCopy2, "2Shadow Copy2", ECommandListType::Copy);
#endif
}

void D3D12TimeManager::ProcessTimeStampHeaps(int count, ID3D12Resource* ResultBuffer, UINT64 ClockFreq, bool IsCopyList, int offset)
{
	D3D12_RANGE readRange = {};
	const D3D12_RANGE emptyRange = {};
	void* pData = nullptr;
	ThrowIfFailed(ResultBuffer->Map(0, &readRange, &pData));
	//D3D12 WARNING: ID3D12CommandQueue::ExecuteCommandLists: Readback Resource (0x00000263E4FEB560:'Unnamed ID3D12Resource Object') still has mapped subresorces when executing a command list that performs a copy operation to the resource.This may be ok if any data read from the readback resources was flushed by calling Unmap() after the resourcecopy operation completed. [ EXECUTION WARNING #927: EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED]

	const UINT64* pTimestamps = reinterpret_cast<UINT64*>(static_cast<UINT8*>(pData) + readRange.Begin);
	for (int i = offset; i < offset + count; i++)
	{
		if (TimeDeltas[i].Used)
		{
			UINT64 delta = pTimestamps[TimeDeltas[i].Endindex] - pTimestamps[TimeDeltas[i].Startindex];
			if (pTimestamps[TimeDeltas[i].Endindex] < pTimestamps[TimeDeltas[i].Startindex])
			{
				Log::LogMessage("Negative GPU timestamps", Log::Warning);
				continue;
			}
			float gpuTimeMS = glm::max(glm::abs((float)(delta * 1000) / (float)ClockFreq), 0.0f);
			TimeDeltas[i].RawTime = gpuTimeMS;
			TimeDeltas[i].avg.Add(gpuTimeMS);

			if (i == 0)
			{
				TimeDeltas[i].StartTS = (pTimestamps[TimeDeltas[i].Startindex]);
				StartTimeStamp = TimeDeltas[i].StartTS;
				if (Device->GetDeviceIndex() == 0)
				{
					GPU0_TS = TimeDeltas[i].StartTS;
				}
				//StartTimeStamp = GPU0_TS;
			}
			else
			{
				const UINT64 CurrentTimeStamp = pTimestamps[TimeDeltas[i].Startindex];
				UINT64 Delta = (pTimestamps[TimeDeltas[i].Startindex] - StartTimeStamp);
				bool IsNegative = false;
				if (CurrentTimeStamp < StartTimeStamp)
				{
					Delta = StartTimeStamp - CurrentTimeStamp;
					IsNegative = true;
				}
				TimeDeltas[i].StartOffset = glm::max((float)(Delta) * 1000 / (float)ClockFreq, 0.0f);
				if (IsNegative)
				{
					TimeDeltas[i].StartOffset = -TimeDeltas[i].StartOffset;
				}
				if (TimeDeltas[i].StartOffset > 1000)
				{
					continue;//ignore large values
				}
				TimeDeltas[i].StartOffsetavg.Add(TimeDeltas[i].StartOffset);
			}
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
	if (Device->GetCpuFrameIndex() == 0)
	{
		ProcessTimeStampHeaps(MaxTimerCount, m_timestampResultBuffers, m_directCommandQueueTimestampFrequencies, false, 0);
		if (m_CopytimestampResultBuffers != nullptr)
		{
			ProcessTimeStampHeaps(EGPUCOPYTIMERS::LIMIT, m_CopytimestampResultBuffers, m_copyCommandQueueTimestampFrequencies, true, CopyOffset);
		}
	}
	AVGgpuTimeMS = TimeDeltas[0].avg.GetCurrentAverage();
	for (int i = 0; i < TotalMaxTimerCount; i++)
	{
		if (!TimeDeltas[i].Used)
		{
			continue;
		}
		PerfManager::Get()->UpdateStat(TimeDeltas[i].Statid, TimeDeltas[i].RawTime, TimeDeltas[i].StartOffsetavg.GetCurrentAverage());
	}
	for (int i = 0; i < TotalMaxTimerCount; i++)
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

void D3D12TimeManager::SetTimerName(int index, std::string Name, ECommandListType::Type type)
{
	if (index >= TotalMaxTimerCount)
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
		TimerData* data = PerfManager::Get()->GetTimerData(TimeDeltas[index].Statid);
		data->name = Name;
		data->IsGPUTimer = true;
		data->TimerType = type;
	}
#if PIX_ENABLED
	PixTimerNames[index] = StringUtils::ConvertStringToWide(Name);
#endif
}
#if PIX_ENABLED
LPCWSTR D3D12TimeManager::GetTimerNameForPIX(int index)
{
	return PixTimerNames[index].c_str();
	}
#endif

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
#if PIX_ENABLED
	//if (/*index == EGPUTIMERS::PointShadows &&*/ !List->IsCopyList())
	{
		PIXBeginEvent(List->GetCommandList(), 0, GetTimerNameForPIX(index));
}
#endif
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
#if PIX_ENABLED
	//if (/*index == EGPUTIMERS::PointShadows &&*/ !List->IsCopyList())
	{

		PIXEndEvent(List->GetCommandList());
}
#endif
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
#if  GPUTIMERS_FULL
		ensure(index < EGPUCOPYTIMERS::LIMIT);
		index += CopyOffset;
		ComandList->EndQuery(m_CopytimestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Startindex);
#endif
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
#if GPUTIMERS_FULL
		ensure(index < EGPUCOPYTIMERS::LIMIT);
		index += CopyOffset;
		ComandList->EndQuery(m_CopytimestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, TimeDeltas[index].Endindex);
#endif
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
	ResolveTimeHeaps(ComandList);
#endif
}

void D3D12TimeManager::EndTotalGPUTimer(ID3D12GraphicsCommandList* ComandList)
{
#if ENABLE_GPUTIMERS
	TimerStarted = false;
	EndTimer(ComandList, 0, false);
#endif
}
void D3D12TimeManager::ResolveTimeHeaps(RHICommandList* CommandList)
{
	((D3D12CommandList*)CommandList)->GetCommandList()->ResolveQueryData(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0, MaxTimerCount * 2, m_timestampResultBuffers, 0);
}

void D3D12TimeManager::ResolveCopyTimeHeaps(RHICommandList* CommandList)
{
#if ENABLE_GPUTIMERS && GPUTIMERS_FULL
	ensure(CommandList->IsCopyList());
	D3D12CommandList* List = (D3D12CommandList*)CommandList;
	List->GetCommandList()->ResolveQueryData(m_CopytimestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0, EGPUCOPYTIMERS::LIMIT * 2, m_CopytimestampResultBuffers, 0);
#endif
}

UINT64 D3D12TimeManager::GPU0_TS = 0;
