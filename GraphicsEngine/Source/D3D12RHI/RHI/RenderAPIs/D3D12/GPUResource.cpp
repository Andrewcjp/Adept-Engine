#include "GPUResource.h"
#include "D3D12DeviceContext.h"
#include "GPUMemoryPage.h"
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
	Device = D3D12RHI::DXConv(device);
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
	ensure(currentState != EResourceState::Evicted);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(Device->GetDevice()->Evict(1, &Pageableresource));
	currentState = EResourceState::Evicted;
}

void GPUResource::MakeResident()
{
	ensure(currentState != EResourceState::Resident);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(Device->GetDevice()->MakeResident(1, &Pageableresource));
	currentState = EResourceState::Resident;
}

bool GPUResource::IsResident()
{
	return (currentState == EResourceState::Resident);
}

EResourceState::Type GPUResource::GetState()
{
	return currentState;
}

void GPUResource::SetResourceState(ID3D12GraphicsCommandList*  List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
#if LOG_RESOURCE_TRANSITIONS
		Log::LogMessage("GPU" + std::to_string(Device->GetDeviceIndex()) + ": Transition: Resource \"" + std::string(GetDebugName()) + "\" From " +
			D3D12Helpers::ResouceStateToString(CurrentResourceState) + " TO " + D3D12Helpers::ResouceStateToString(newstate));
#endif
		List->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, CurrentResourceState, newstate));
		CurrentResourceState = newstate;
		TargetState = newstate;
	}
}
//todo More Detailed Error checking!
void GPUResource::StartResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		BarrierDesc.Transition.StateBefore = CurrentResourceState;
		BarrierDesc.Transition.StateAfter = newstate;
		BarrierDesc.Transition.pResource = resource;
		List->ResourceBarrier(1, &BarrierDesc);
		TargetState = newstate;
	}
}

void GPUResource::EndResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		BarrierDesc.Transition.StateBefore = CurrentResourceState;
		BarrierDesc.Transition.StateAfter = newstate;
		BarrierDesc.Transition.pResource = resource;
		List->ResourceBarrier(1, &BarrierDesc);
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

D3D12_RESOURCE_STATES GPUResource::GetCurrentState()
{
	return CurrentResourceState;
}

ID3D12Resource * GPUResource::GetResource()
{
	return resource;
}

void GPUResource::Release()
{
	IRHIResourse::Release();
	if (resource != nullptr)
	{
#if 0
		int iirefcount = resource->AddRef();
		int niirefcount = resource->Release();
#endif
		//there is a resource contention issue here with gpu 0 and 1 
		//where GPU will move forward and delete before GPU 1 has finished with resource.
		resource->Release();
		resource = nullptr;
	}
	if (Page != nullptr)
	{
		Page->Deallocate(this);
	}
	//SafeRelease(resource);
	RemoveCheckerRef(GPUResource, this);
}


