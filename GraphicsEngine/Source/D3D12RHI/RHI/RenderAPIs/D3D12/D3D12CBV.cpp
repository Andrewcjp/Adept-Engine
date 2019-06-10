
#include "D3D12CBV.h"
#include "D3D12RHI.h"
#if BUILD_D3D12
#include "D3D12DeviceContext.h"
#include "DescriptorHeapManager.h"
#include "DXDescriptor.h"
D3D12CBV::D3D12CBV(DeviceContext* inDevice)
{
	Device = D3D12RHI::DXConv(inDevice);
	if (Device == nullptr)
	{
		Device = D3D12RHI::DXConv(RHI::GetDeviceContext());
	}
}


D3D12CBV::~D3D12CBV()
{
	if (m_constantBuffer)
	{
		CD3DX12_RANGE readRange(0, 0);
		m_constantBuffer->Unmap(0, &readRange);
		m_constantBuffer->Release();
	}
}

void D3D12CBV::SetGpuView(ID3D12GraphicsCommandList * list, int offset, int slot, bool IsCompute)
{
	if (IsCompute)
	{
		list->SetComputeRootConstantBufferView(slot, m_constantBuffer->GetGPUVirtualAddress() + (offset * CB_Size));
	}
	else
	{
		list->SetGraphicsRootConstantBufferView(slot, m_constantBuffer->GetGPUVirtualAddress() + (offset * CB_Size));
	}
}

void D3D12CBV::UpdateCBV(void* buffer, int offset, int size)
{
	//access volition
	ensure((offset*CB_Size) + size <= SizeInBytes);
	memcpy(m_pCbvDataBegin + (offset * RawStuctSize), buffer, size);
}

void D3D12CBV::InitCBV(int StructSize, int Elementcount)
{
	InitalBufferCount = Elementcount;
	RawStuctSize = StructSize;
	CB_Size = (StructSize + 255) & ~255;
	RawStuctSize = CB_Size;
	SizeInBytes = InitalBufferCount * CB_Size;
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(SizeInBytes),//1024 * 64
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer)));

	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
#if 0//validate CBV
	int DataSize = 1;
	for (int i = 0; i < SizeInBytes; i++)
	{
		unsigned char y = 10;
		memcpy(m_pCbvDataBegin + i * DataSize, &y, DataSize);
	}
#endif
}

void D3D12CBV::SetName(LPCWSTR name)
{
	m_constantBuffer->SetName(name);
}
#endif