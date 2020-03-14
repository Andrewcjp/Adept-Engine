#include "GPUResource.h"
#include "D3D12DeviceContext.h"
#include "GPUMemoryPage.h"
#include "Core\Performance\PerfManager.h"
#include "D3D12CommandList.h"
//todo: More Detailed Error checking!
CreateChecker(GPUResource);
GPUResource::GPUResource()
{}

GPUResource::GPUResource(ID3D12Resource* Target, D3D12_RESOURCE_STATES InitalState) :GPUResource(Target, InitalState, RHI::GetDefaultDevice())
{}

GPUResource::GPUResource(ID3D12Resource * Target, D3D12_RESOURCE_STATES InitalState, DeviceContext * device)
{
	AddCheckerRef(GPUResource, this);
	resource = Target;
	SetName(L"GPU Resource");
	SetDebugName("GPU Resource");
	CurrentResourceState = InitalState;
	TargetState = CurrentResourceState;
	Device = D3D12RHI::DXConv(device);
	PerfManager::Get()->AddTimer("ResourceTransitons", "RHI");
}

GPUResource::~GPUResource()
{
	if (!IsReleased)
	{
		Release();
	}
}

void GPUResource::SetName(LPCWSTR name)
{
	resource->SetName(name);
}

void GPUResource::Evict()
{
	ensure(currentState != EResourcePageState::Evicted);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(Device->GetDevice()->Evict(1, &Pageableresource));
	currentState = EResourcePageState::Evicted;
}

void GPUResource::MakeResident()
{
	ensure(currentState != EResourcePageState::Resident);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(Device->GetDevice()->MakeResident(1, &Pageableresource));
	currentState = EResourcePageState::Resident;
}

bool GPUResource::IsResident()
{
	return (currentState == EResourcePageState::Resident);
}

EResourcePageState::Type GPUResource::GetState()
{
	return currentState;
}

bool GPUResource::IsValidStateForList(D3D12CommandList* List)
{
	if (CurrentResourceState == D3D12_RESOURCE_STATE_COMMON)
	{
		return true;
	}
	if (List->IsGraphicsList())
	{
		return true;
	}
	if (List->IsComputeList() || List->IsRaytracingList())
	{
		return CurrentResourceState | ~D3D12_RESOURCE_STATE_DEPTH_WRITE || CurrentResourceState | ~D3D12_RESOURCE_STATE_RENDER_TARGET;
	}
	if (List->IsCopyList())
	{

	}
	return false;
}

void GPUResource::SetResourceState(D3D12CommandList*  List, D3D12_RESOURCE_STATES newstate, EResourceTransitionMode::Type Mode/* = EResourceTransitionMode::Direct*/)
{
	if (newstate != CurrentResourceState)
	{
#if LOG_RESOURCE_TRANSITIONS
		Log::LogMessage("GPU" + std::to_string(Device->GetDeviceIndex()) + ": Transition: Resource \"" + std::string(GetDebugName()) + "\" From " +
			D3D12Helpers::ResouceStateToString(CurrentResourceState) + " TO " + D3D12Helpers::ResouceStateToString(newstate));
#endif
		if (Mode == EResourceTransitionMode::Direct)
		{
			ensure(!IsTransitioning());
			List->AddTransition(CD3DX12_RESOURCE_BARRIER::Transition(resource, CurrentResourceState, newstate));
			CurrentResourceState = newstate;
			TargetState = newstate;
		}
		else if (Mode == EResourceTransitionMode::Start)
		{
			ensure(!IsTransitioning());
			StartResourceTransition(List, newstate);
		}
		else if (Mode == EResourceTransitionMode::End)
		{
			ensure(IsTransitioning());
			EndResourceTransition(List, newstate);
		}

		PerfManager::Get()->AddToCountTimer("ResourceTransitons", 1);
	}
}

void GPUResource::StartResourceTransition(D3D12CommandList * List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		BarrierDesc.Transition.StateBefore = CurrentResourceState;
		BarrierDesc.Transition.StateAfter = newstate;
		BarrierDesc.Transition.pResource = resource;
		List->AddTransition(BarrierDesc);
		TargetState = newstate;
	}
}

void GPUResource::EndResourceTransition(D3D12CommandList * List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		BarrierDesc.Transition.StateBefore = CurrentResourceState;
		BarrierDesc.Transition.StateAfter = newstate;
		BarrierDesc.Transition.pResource = resource;
		List->AddTransition(BarrierDesc);
		CurrentResourceState = newstate;
	}
}
bool GPUResource::IsTransitioning()
{
	return (CurrentResourceState != TargetState);
}

void GPUResource::SetGPUPage(GPUMemoryPage * page)
{
	Page = page;
}

void GPUResource::UpdateUnTrackedState(D3D12_RESOURCE_STATES newstate)
{
	CurrentResourceState = newstate;
	TargetState = newstate;
}

inline EPhysicalMemoryState::Type GPUResource::GetCurrentAliasState() const { return CurrentAliasState; }

inline void GPUResource::SetCurrentAliasState(EPhysicalMemoryState::Type val) { CurrentAliasState = val; }

bool GPUResource::NeedsClear() const
{
	return CurrentAliasState == EPhysicalMemoryState::Active_NoClear;
}

void GPUResource::NotifyClearComplete()
{
	ensure(NeedsClear());
	CurrentAliasState = EPhysicalMemoryState::Active_NoClear;
}

D3D12_RESOURCE_STATES GPUResource::GetCurrentState()
{
	ensure(!IsTransitioning());
	return CurrentResourceState;
}

ID3D12Resource * GPUResource::GetResource()
{
	return resource;
}

void GPUResource::Release()
{
	IRHIResourse::Release();
	if (Page != nullptr)
	{
		Page->Deallocate(this);
	}
	//if the driver crashes here then (most likely) there is a resource contention issue with gpu 0 and 1 
	//where GPU will move forward and delete resources before GPU 1 has finished with resource.
	SafeRelease(resource);
	RemoveCheckerRef(GPUResource, this);
}


