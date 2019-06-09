#include "D3D12RHIPCH.h"
#include "GPUMemoryPage.h"
#include "Core/Utils/StringUtil.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"

GPUMemoryPage::GPUMemoryPage(AllocDesc & desc, D3D12DeviceContext* context)
{
	PageDesc = desc;
	Device = context;
}

GPUMemoryPage::~GPUMemoryPage()
{
	MemoryUtils::DeleteReleaseableVector(ContainedResources);
}

EAllocateResult::Type GPUMemoryPage::Allocate(AllocDesc & desc, GPUResource** Resource)
{
	//DXMM: Todo Placed resource 
	ID3D12Resource* DxResource = nullptr;
	std::wstring Conv = StringUtils::ConvertStringToWide(desc.Name);
	D3D12Helpers::AllocateUAVBuffer(Device->GetDevice(), desc.Size, &DxResource, desc.InitalState, Conv.c_str());
	*Resource = new GPUResource(DxResource, desc.InitalState, Device);
	(*Resource)->SetDebugName(desc.Name);
	ContainedResources.push_back(*Resource);
	return EAllocateResult::OK;
}

void GPUMemoryPage::Compact()
{}

void GPUMemoryPage::Deallocate(GPUResource * R)
{
	VectorUtils::Remove(ContainedResources, R);
}

UINT GPUMemoryPage::GetSize() const
{
	return PageDesc.Size;
}
