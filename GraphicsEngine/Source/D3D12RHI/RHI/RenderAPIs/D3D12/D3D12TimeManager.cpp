#include "D3D12TimeManager.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h" 
#if PIX_ENABLED
#define PROFILE_BUILD 
#include "pix3.h"
#endif
#include "D3D12QueryHeap.h"
#include "D3D12Query.h"

D3D12TimeManager::D3D12TimeManager(DeviceContext* context) :RHITimeManager(context)
{
	Init(context);
}

D3D12TimeManager::~D3D12TimeManager()
{}

void D3D12TimeManager::Init(DeviceContext* context)
{
	Device = D3D12RHI::DXConv(context);
#if ENABLE_GPUTIMERS
	UpdateTimeStampFreq();

	std::string GPUName = "GPU_";
	GPUName.append(std::to_string(context->GetDeviceIndex()));
	StatsGroupId = PerfManager::Get()->GetGroupId(GPUName.c_str());

	SetTimerName(EGPUTIMERS::Total, "Total GPU");
	SetTimerName(EGPUTIMERS::MainPass, "Main Pass");
	SetTimerName(EGPUTIMERS::DeferredWrite, "Deferred Write");
	SetTimerName(EGPUTIMERS::DeferredLighting, "Deferred Lighting");
	SetTimerName(EGPUTIMERS::PreZ, "Pre-Z");
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
	SetTimerName(EGPUTIMERS::CubemapCapture, "Cubemap Capture");
	SetTimerName(EGPUTIMERS::DebugRender, "Debug Render");
	SetTimerName(EGPUTIMERS::RT_Trace, "RayTrace", ECommandListType::RayTracing);
	SetTimerName(EGPUCOPYTIMERS::MGPUCopy, "MGPU Copy", ECommandListType::Copy);
	SetTimerName(EGPUCOPYTIMERS::SFRMerge, "SFR Merge", ECommandListType::Copy);
	SetTimerName(EGPUCOPYTIMERS::ShadowCopy, "Shadow Copy", ECommandListType::Copy);
	SetTimerName(EGPUCOPYTIMERS::ShadowCopy2, "2Shadow Copy2", ECommandListType::Copy);
#endif
	UpdateTimeStampFreq();
#if AFTERMATH
	GFSDK_Aftermath_Result result = GFSDK_Aftermath_DX12_Initialize(GFSDK_Aftermath_Version_API, GFSDK_Aftermath_FeatureFlags_Maximum, Device->GetDevice());
	ensure(result == GFSDK_Aftermath_Result_Success);
#endif
}

void D3D12TimeManager::UpdateTimeStampFreq()
{
	ThrowIfFailed(Device->GetCommandQueue()->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies));
	ThrowIfFailed(Device->GetCommandQueueFromEnum(DeviceContextQueue::Compute)->GetTimestampFrequency(&m_ComputeQueueFreqency));
#if GPUTIMERS_FULL
	if (Device->GetCaps().SupportsCopyTimeStamps)
	{
		ThrowIfFailed(Device->GetCommandQueueFromEnum(DeviceContextQueue::Copy)->GetTimestampFrequency(&m_copyCommandQueueTimestampFrequencies));
	}
#endif
}

void D3D12TimeManager::UpdateTimers()
{
#if ENABLE_GPUTIMERS
	UpdateTimeStampFreq();
	Device->GetTimeStampHeap()->ReadData();
	ResolveAllTimers();
	for (int i = 0; i < TimerQueries.size(); i++)
	{
		if (TimerQueries[i].TimerQueries.size() < 2)
		{
			continue;
		}
		if (TimerQueries[i].name == "Total GPU")
		{
			StartTimeStamp = TimerQueries[i].TimerQueries[0]->Result;
			break;
		}
	}
	for (int i = 0; i < TimerQueries.size(); i++)
	{
		TimerQ* Q = &TimerQueries[i];
		if (Q->TimerQueries.size() < 2)
		{
			Q->TimerQueries.clear();
			continue;
		}
		int id = PerfManager::Get()->AddGPUTimer((Q->name + std::to_string(Device->GetDeviceIndex())).c_str(), StatsGroupId);
		float Offset = 0.0f;
		if (Q->name == "Total GPU")
		{
			StartTimeStamp = TimerQueries[i].TimerQueries[0]->Result;
			AVGgpuTimeMS = PerfManager::Get()->GetTimerData(id)->AVG->GetCurrentAverage();
		}
		else
		{
			Offset = ConvertTimeStampToMS(TimerQueries[i].TimerQueries[0]->Result - StartTimeStamp);
		}
		GPUTimer* GT = RHITimeManager::GetTimer(Q->name, Device);
		if (GT != nullptr)
		{
			GT->Clear();
			for (D3D12Query* Q : TimerQueries[i].TimerQueries)
			{
				GT->AddResults(Q->Result);
			}			
		}
		PerfManager::Get()->UpdateStat(id, Q->TotalTime, Offset);
		Q->TimerQueries.clear();
	}
	PushToPerfManager();
#endif
}

float D3D12TimeManager::ConvertTimeStampToMS(long Time)
{
	UINT64 Delta = Time;
	return  (float)(Delta * 1000 / m_directCommandQueueTimestampFrequencies);
}

void D3D12TimeManager::SetTimerName(int index, std::string Name, ECommandListType::Type type)
{
	if (index >= TotalMaxTimerCount)
	{
		return;
	}
	ensure(index > -1);
	TimerNames[index] = Name;
	MaxIndexInUse = std::max(index, MaxIndexInUse);
	int Statid = PerfManager::Get()->GetTimerIDByName(Name + std::to_string(Context->GetDeviceIndex()));
	PerfManager::Get()->AddTimer(Statid, StatsGroupId);
	if (PerfManager::Get()->GetTimerData(Statid) != nullptr)
	{
		TimerData* data = PerfManager::Get()->GetTimerData(Statid);
		data->name = Name;
		data->IsGPUTimer = true;
		data->TimerType = type;
	}
#if PIX_ENABLED
	PixTimerNames[index] = Name;
#endif
}
#if PIX_ENABLED
const char* D3D12TimeManager::GetTimerNameForPIX(int index)
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
	D3D12CommandList* List = D3D12RHI::DXConv(CommandList);
	GetOrCreateTimer(GetTimerName(index), CommandList->GetDevice(), CommandList->GetListType());
	StartTimer(List, index, List->IsCopyList());
#if PIX_ENABLED
	//if (/*index == EGPUTIMERS::PointShadows &&*/ !List->IsCopyList())
	{
		//PIXSetMarker(0, GetTimerNameForPIX(index));
		PIXBeginEvent(List->GetCommandList(), index, GetTimerNameForPIX(index));
	}
#endif
#if AFTERMATH
	const char* t = TimeDeltas[index].name.c_str();
	GFSDK_Aftermath_SetEventMarker(List->AMHandle, &(t), TimeDeltas[index].name.size());
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
	D3D12CommandList* List = D3D12RHI::DXConv(CommandList);
	EndTimer(List, index, List->IsCopyList());
#if PIX_ENABLED
	PIXEndEvent(List->GetCommandList());
#endif
#endif
}

float D3D12TimeManager::GetTotalTime()
{
	return AVGgpuTimeMS;
}

void D3D12TimeManager::StartTimer(D3D12CommandList * ComandList, int index, bool IsCopy)
{
	TimerQ* timer = GetTimer(TimerNames[index]);
	if (timer == nullptr)
	{
		TimerQ q;
		q.name = TimerNames[index];
		TimerQueries.push_back(q);
		timer = &TimerQueries[TimerQueries.size() - 1];
	}
	ensure(timer);
	D3D12Query* Query = D3D12RHI::DXConv(RHI::CreateQuery(EGPUQueryType::Timestamp, Device));
	if (IsCopy)
	{
		Device->GetCopyTimeStampHeap()->EndQuerry(ComandList, Query);
	}
	else
	{
		Device->GetTimeStampHeap()->EndQuerry(ComandList, Query);
	}
	timer->TimerQueries.push_back(Query);
}

void D3D12TimeManager::EndTimer(D3D12CommandList* ComandList, int index, bool IsCopy)
{
	TimerQ* timer = GetTimer(TimerNames[index]);
	if (timer == nullptr)
	{
		TimerQ q;
		q.name = TimerNames[index];
		TimerQueries.push_back(q);
		timer = &TimerQueries[TimerQueries.size() - 1];
	}
	ensure(timer);
	D3D12Query* Query = D3D12RHI::DXConv(RHI::CreateQuery(EGPUQueryType::Timestamp, Device));
	timer->TimerQueries.push_back(Query);
	if (IsCopy)
	{
		Device->GetCopyTimeStampHeap()->EndQuerry(ComandList, Query);
	}
	else
	{
		Device->GetTimeStampHeap()->EndQuerry(ComandList, Query);
	}
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
	if (TimerStarted)
	{
		EndTimer(ComandList, 0);
		TimerStarted = false;
	}
	Device->GetTimeStampHeap()->ResolveAndEndQueryBatches(D3D12RHI::DXConv(ComandList));
	//Device->GetCopyTimeStampHeap()->ResolveAndEndQueryBatches(D3D12RHI::DXConv(ComandList));
#endif
}

void D3D12TimeManager::TimerQ::Resolve(UINT64 freqnecy)
{
	if (TimerQueries.size() < 2)
	{
		TotalTime = 0.0f;
		return;
	}
	TotalTime = 0.0f;
	for (int i = 0; i < TimerQueries.size(); i += 2)
	{
		if (TimerQueries[i]->IsResolved && TimerQueries[i + 1]->IsResolved)
		{
			UINT64 Delta = TimerQueries[i + 1]->Result - TimerQueries[i]->Result;
			//resolve "backwards" timers  (also prevents UINT64 max showing in Performance timers)
			if (TimerQueries[i]->Result > TimerQueries[i + 1]->Result)
			{
				continue;
				Delta = TimerQueries[i]->Result - TimerQueries[i + 1]->Result;
			}
			float time = Delta * 1000 / (double)freqnecy;
			TotalTime += time;
		}
	}
}

D3D12TimeManager::TimerQ* D3D12TimeManager::GetTimer(std::string name)
{
	//int map
	for (int i = 0; i < TimerQueries.size(); i++)
	{
		if (TimerQueries[i].name == name)
		{
			return &TimerQueries[i];
		}
	}
	return nullptr;
}

void D3D12TimeManager::ResolveAllTimers()
{
	for (int i = 0; i < TimerQueries.size(); i++)
	{
		TimerQueries[i].Resolve(m_directCommandQueueTimestampFrequencies);
		//update
	}
}
