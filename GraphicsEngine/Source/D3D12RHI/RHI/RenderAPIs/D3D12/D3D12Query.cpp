#include "D3D12RHIPCH.h"
#include "D3D12Query.h"


D3D12Query::D3D12Query(EGPUQueryType::Type Type, DeviceContext * device) :RHIQuery(Type, device)
{

}

D3D12Query::~D3D12Query()
{}
