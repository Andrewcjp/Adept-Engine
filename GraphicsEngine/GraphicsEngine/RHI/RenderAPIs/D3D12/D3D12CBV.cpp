#include "stdafx.h"
#include "D3D12CBV.h"
#include "D3D12RHI.h"
#include "EngineGlobals.h"
#include "RHI/DeviceContext.h"
#if BUILD_D3D12
D3D12CBV::D3D12CBV(DeviceContext* inDevice)
{
	Device = inDevice;
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDeviceContext();
	}
}


D3D12CBV::~D3D12CBV()
{
	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
	}
	if (m_cbvHeap)
	{
		m_cbvHeap->Release();
	}
}
void D3D12CBV::SetDescriptorHeaps(ID3D12GraphicsCommandList* list)
{
	//assert(offset < InitalBufferCount && "Out of Buffers");
	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	
}
void D3D12CBV::SetGpuView(ID3D12GraphicsCommandList * list, int offset, int slot)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE  cbvSrvHandle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	list->SetGraphicsRootConstantBufferView(slot, m_constantBuffer->GetGPUVirtualAddress() + (offset * CB_Size));
}

void D3D12CBV::InitCBV(int StructSize, int Elementcount)
{
	InitalBufferCount = Elementcount;
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(Device->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

	CB_Size = (StructSize + 255) & ~255;
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(InitalBufferCount * CB_Size),//1024 * 64
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer)));

	// Describe and create a constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	
	cbvDesc.SizeInBytes = CB_Size;	// CB size is required to be 256-byte aligned.
	Device->GetDevice()->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	//memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	std::wstring str = L"CBV ";
	str.append(std::to_wstring(StructSize));
	//str.forma
	m_cbvHeap->SetName(str.c_str());
}
#endif