#include "D3D12Buffer.h"
#include "Core/Utils/RefChecker.h"
#include "RHI/DeviceContext.h"
#include "D3D12DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "GPUResource.h"
#include "DescriptorGroup.h"
#include "D3D12CBV.h"
#include "D3D12CommandList.h"
#include "DescriptorHeapManager.h"

D3D12Buffer::D3D12Buffer(ERHIBufferType::Type type, DeviceContext * inDevice) :RHIBuffer(type)
{
	AddCheckerRef(D3D12Buffer, this);
	if (inDevice == nullptr)
	{
		Device = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	}
	else
	{
		Device = D3D12RHI::DXConv(inDevice);
	}
	Context = Device;
}

void D3D12Buffer::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12Buffer, this);
	Device = nullptr;
	if (CurrentBufferType == ERHIBufferType::Constant)
	{
		for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
		{
			MemoryUtils::DeleteCArray(CBV[i], RHI::CPUFrameCount);
		}
	}
	SafeRelease(m_DataBuffer);
	SafeRelease(SRVDesc);
	SafeRelease(UAV);
}

D3D12Buffer::~D3D12Buffer()
{}

void D3D12Buffer::CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices)
{
	ensure(StructSize > 0);
	ensure(Elementcount > 0);
	TotalByteSize = StructSize;
	CrossDevice = ReplicateToAllDevices;
	if (ReplicateToAllDevices)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			for (int j = 0; j < RHI::CPUFrameCount; j++)
			{
				CBV[i][j] = new D3D12CBV(RHI::GetDeviceContext(i));
				CBV[i][j]->InitCBV(StructSize, Elementcount);
				D3D12Helpers::NameRHIObject(CBV[i][j], this);
			}
		}
	}
	else
	{
		for (int j = 0; j < RHI::CPUFrameCount; j++)
		{
			CBV[0][j] = new D3D12CBV(Device);
			CBV[0][j]->InitCBV(StructSize, Elementcount);
			D3D12Helpers::NameRHIObject(CBV[0][j], this);
		}
	}
}

void D3D12Buffer::UpdateConstantBuffer(void * data, int offset)
{
	const int index = Device->GetCpuFrameIndex();
	if (CrossDevice)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			if (RHI::GetFrameCount() == 0)
			{
				CBV[i][0]->UpdateCBV(data, offset, TotalByteSize);
				CBV[i][1]->UpdateCBV(data, offset, TotalByteSize);
			}
			else
			{
				CBV[i][index]->UpdateCBV(data, offset, TotalByteSize);
			}
		}
	}
	else
	{
		if (RHI::GetFrameCount() == 0)
		{
			CBV[0][0]->UpdateCBV(data, offset, TotalByteSize);
			CBV[0][1]->UpdateCBV(data, offset, TotalByteSize);
		}
		else
		{
			CBV[0][index]->UpdateCBV(data, offset, TotalByteSize);
		}
	}
}

void D3D12Buffer::SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Slot, bool  IsCompute, int Deviceindex)
{
	const int index = Device->GetCpuFrameIndex();
	if (CrossDevice)
	{
		CBV[Deviceindex][index]->SetGpuView(list, offset, Slot, IsCompute);
	}
	else
	{
		CBV[0][index]->SetGpuView(list, offset, Slot, IsCompute);
	}
}

GPUResource * D3D12Buffer::GetResource()
{
	return m_DataBuffer;
}

DescriptorGroup * D3D12Buffer::GetDescriptor()
{
	return SRVDesc;
}

void D3D12Buffer::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype)
{
	BufferAccesstype = Accesstype;
	if (BufferAccesstype == EBufferAccessType::Dynamic)
	{
		CreateDynamicBuffer(ByteSize);
	}
	else if (BufferAccesstype == EBufferAccessType::Static)
	{
		CreateStaticBuffer(ByteSize);
	}
	m_vertexBufferView.BufferLocation = m_DataBuffer->GetResource()->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = Stride;
	m_vertexBufferView.SizeInBytes = TotalByteSize;
	D3D12Helpers::NameRHIObject(m_DataBuffer, this);
	ElementCount = TotalByteSize / Stride;
	ElementSize = Stride;
}

void D3D12Buffer::UpdateVertexBuffer(void * data, size_t length)
{
	VertexCount = length;
	UpdateData(data, length, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	if (BufferAccesstype == EBufferAccessType::Dynamic)
	{
		PostUploadState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
}

void D3D12Buffer::BindBufferReadOnly(RHICommandList * list, int RSSlot)
{
	SetupBufferSRV();
	D3D12CommandList* d3dlist = D3D12RHI::DXConv(list);
	if (BufferAccesstype != EBufferAccessType::GPUOnly && BufferAccesstype != EBufferAccessType::Dynamic)//gpu buffer states are explicitly managed by render code
	{
		m_DataBuffer->SetResourceState(d3dlist->GetCommandList(), PostUploadState);
	}
	if (list->IsComputeList())
	{
		d3dlist->GetCommandList()->SetComputeRootDescriptorTable(RSSlot, SRVDesc->GetGPUAddress(0));
	}
	else
	{
		d3dlist->GetCommandList()->SetGraphicsRootDescriptorTable(RSSlot, SRVDesc->GetGPUAddress(0));
	}
}

void D3D12Buffer::SetBufferState(RHICommandList * list, EBufferResourceState::Type State)
{
	D3D12CommandList* d3dlist = D3D12RHI::DXConv(list);
	m_DataBuffer->SetResourceState(d3dlist->GetCommandList(), D3D12Helpers::ConvertBufferResourceState(State));
}

void D3D12Buffer::SetupBufferSRV()
{
	if (SRVDesc == nullptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		if (BufferAccesstype == EBufferAccessType::GPUOnly)
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		else
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		}

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = ElementCount;
		srvDesc.Buffer.StructureByteStride = ElementSize;
		if (CurrentBufferType == ERHIBufferType::Index)
		{
			srvDesc.Format = DXGI_FORMAT_R16_UINT;
			srvDesc.Buffer.StructureByteStride = 0;
		}
		SRVDesc = Device->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		SRVDesc->CreateShaderResourceView(m_DataBuffer->GetResource(), &srvDesc);
	}
}

void D3D12Buffer::CreateUAV()
{
	UAV = RHI::CreateUAV();
	UAV->CreateUAVFromRHIBuffer(this);
}

void D3D12Buffer::UpdateData(void * data, size_t length, D3D12_RESOURCE_STATES EndState)
{
	PostUploadState = EndState;
	if (BufferAccesstype == EBufferAccessType::Dynamic)
	{
		UINT8* pVertexDataBegin;
		MapBuffer(reinterpret_cast<void**>(&pVertexDataBegin));
		//#DX12 invalid call near here?
		memcpy(pVertexDataBegin, data, length);
		UnMap();
	}
	else
	{
		ensure(!UploadComplete && "Uploading More than once to a GPU only buffer is not allowed!");
		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA Data = {};
		Data.pData = reinterpret_cast<BYTE*>(data); // pointer to our index array
		Data.RowPitch = TotalByteSize; // size of all our index buffer
		Data.SlicePitch = TotalByteSize; // also the size of our index buffer

											// we are now creating a command with the command list to copy the data from
											// the upload heap to the default heap
		UpdateSubresources(Device->GetCopyList(), m_DataBuffer->GetResource(), m_UploadBuffer, 0, 0, 1, &Data);

		UploadComplete = true;
		Device->NotifyWorkForCopyEngine();
		D3D12RHI::Instance->AddObjectToDeferredDeleteQueue(m_UploadBuffer);
	}
}

bool D3D12Buffer::CheckDevice(int index)
{
	if (CurrentBufferType == ERHIBufferType::Constant && CrossDevice)
	{
		//ready on all devices!
		return true;
	}
	if (Device != nullptr)
	{
		return (Device->GetDeviceIndex() == index);
	}
	return false;
}

void D3D12Buffer::EnsureResouceInFinalState(ID3D12GraphicsCommandList * list)
{
	m_DataBuffer->SetResourceState(list, PostUploadState);
}

void D3D12Buffer::CreateStaticBuffer(int ByteSize)
{
	TotalByteSize = ByteSize;//max size
	ID3D12Resource* TempRes = nullptr;
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&TempRes)));
	m_DataBuffer = new GPUResource(TempRes, D3D12_RESOURCE_STATE_COPY_DEST, Device);
	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at

	// create upload heap to upload index buffer
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)));
	D3D12Helpers::NameRHIObject(m_UploadBuffer, this, "(UPLOAD)");
}

void D3D12Buffer::CreateDynamicBuffer(int ByteSize)
{
	//This Buffer Will Have Data Changed Every frame so no need to transition to only gpu.
	// Create the vertex buffer.
	TotalByteSize = ByteSize;
	ID3D12Resource* TempRes = nullptr;
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&TempRes)));
	m_DataBuffer = new GPUResource(TempRes, D3D12_RESOURCE_STATE_GENERIC_READ, Device);
	PostUploadState = D3D12_RESOURCE_STATE_GENERIC_READ;
}
static inline UINT AlignForUavCounter(UINT bufferSize)
{
	const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
	return (bufferSize + (alignment - 1)) & ~(alignment - 1);
}
void D3D12Buffer::CreateBuffer(RHIBufferDesc desc)
{
	ElementSize = desc.Stride;
	ElementCount = desc.ElementCount;
	BufferAccesstype = desc.Accesstype;
	if (desc.CounterSize > 0)
	{
		CounterOffset = desc.ElementCount * desc.Stride;
		CounterOffset = AlignForUavCounter(CounterOffset);
	}
	Desc = desc;
	TotalByteSize = desc.ElementCount * desc.Stride + desc.CounterSize;
	if (BufferAccesstype == EBufferAccessType::GPUOnly)
	{
		ID3D12Resource* TempRes = nullptr;
		ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE),
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&TempRes)));
		m_DataBuffer = new GPUResource(TempRes, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, Device);
	}
	else if (BufferAccesstype == EBufferAccessType::Dynamic)
	{
		CreateDynamicBuffer(TotalByteSize);
	}
	else if (BufferAccesstype == EBufferAccessType::Static)
	{
		CreateStaticBuffer(TotalByteSize);
	}
	if (desc.CreateSRV)
	{
		SetupBufferSRV();
	}
	if (desc.CreateUAV)
	{
		CreateUAV();
	}
	D3D12Helpers::NameRHIObject(m_DataBuffer, this);
}

void D3D12Buffer::UpdateIndexBuffer(void * data, size_t length)
{
	VertexCount = length;
	UpdateData(data, length, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void D3D12Buffer::UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state)
{
	UpdateData(data, length, D3D12Helpers::ConvertBufferResourceState(state));
}

void D3D12Buffer::CreateIndexBuffer(int Stride, int ByteSize)
{
	TotalByteSize = ByteSize;
	CreateStaticBuffer(ByteSize);
	m_IndexBufferView.BufferLocation = m_DataBuffer->GetResource()->GetGPUVirtualAddress();
#if USE_16BIT_INDICIES
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
#else
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
#endif

	m_IndexBufferView.SizeInBytes = TotalByteSize;
	D3D12Helpers::NameRHIObject(m_DataBuffer, this);
	ElementCount = TotalByteSize / Stride;
	ElementSize = Stride;
}

void D3D12Buffer::MapBuffer(void ** Data)
{
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_DataBuffer->GetResource()->Map(0, &readRange, Data));
}

void D3D12Buffer::UnMap()
{
	m_DataBuffer->GetResource()->Unmap(0, nullptr);
}


