#include "D3D12Buffer.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "D3D12Framebuffer.h"
#include "D3D12Helpers.h"
#include "D3D12InterGPUStagingResource.h"
#include "DescriptorHeap.h"
#include "DescriptorHeapManager.h"
#include "DXDescriptor.h"
#include "DXMemoryManager.h"
#include "GPUResource.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"

D3D12Buffer::D3D12Buffer(ERHIBufferType::Type type, DeviceContext* inDevice) :RHIBuffer(type)
{
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
	Device = nullptr;
	SafeRelease(m_DataBuffer);
	SafeRelease(m_DataBufferDouble[0]);
	SafeRelease(m_DataBufferDouble[1]);
}

D3D12Buffer::~D3D12Buffer()
{
}

void D3D12Buffer::CreateConstantBuffer(int iStructSize, int iElementcount, bool ReplicateToAllDevices)
{
	ensure(iStructSize > 0);
	ensure(iElementcount > 0);
	InitCBV(iStructSize, iElementcount);
}

void D3D12Buffer::UpdateConstantBuffer(void* data, int offset)
{
	ensure((offset * StructSize) + StructSize <= TotalByteSize);
	memcpy(m_pCbvDataBegin[Context->GetCpuFrameIndex()] + (offset * StructSize), data, RawStructSize);
	if (RHI::GetFrameCount() == 0)
	{
		memcpy(m_pCbvDataBegin[1] + (offset * StructSize), data, RawStructSize);
	}
}

void D3D12Buffer::InitCBV(int iStructSize, int Elementcount)
{
	RawStructSize = iStructSize;
	StructSize = (iStructSize + 255) & ~255;
	TotalByteSize = StructSize * Elementcount;
	AllocDesc desc = {};
	desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize);

	desc.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	Device->GetMemoryManager()->AllocUploadTemporary(desc, &m_DataBufferDouble[0]);
	Device->GetMemoryManager()->AllocUploadTemporary(desc, &m_DataBufferDouble[1]);

	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_DataBufferDouble[0]->GetResource()->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin[0])));
	ThrowIfFailed(m_DataBufferDouble[1]->GetResource()->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin[1])));
}

void D3D12Buffer::SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Slot, bool  IsCompute, int Deviceindex)
{
	if (IsCompute)
	{
		list->SetComputeRootConstantBufferView(Slot, GetDoubleBuffer()->GetResource()->GetGPUVirtualAddress() + (offset * StructSize));
	}
	else
	{
		list->SetGraphicsRootConstantBufferView(Slot, GetDoubleBuffer()->GetResource()->GetGPUVirtualAddress() + (offset * StructSize));
	}
}

GPUResource* D3D12Buffer::GetResource()
{
	return m_DataBuffer;
}


DXDescriptor* D3D12Buffer::GetDescriptor(const RHIViewDesc& desc, DescriptorHeap* heap)
{
	if (heap == nullptr)
	{
		heap = Device->GetHeapManager()->GetMainHeap();
	}
	ensure(desc.ViewType != EViewType::Limit);
	DXDescriptor* Descriptor = heap->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	if (desc.ViewType == EViewType::SRV)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = ElementCount;
		srvDesc.Buffer.StructureByteStride = ElementSize;
		srvDesc.Buffer.FirstElement = desc.FirstElement;
		if (desc.Format == ETextureFormat::FORMAT_UNKNOWN)
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Buffer.StructureByteStride = ElementSize;
		}
		else
		{
			srvDesc.Format = D3D12Helpers::ConvertFormat(desc.Format);
			srvDesc.Buffer.StructureByteStride = 0;
		}
		if (CurrentBufferType == ERHIBufferType::Index)
		{
			srvDesc.Format = DXGI_FORMAT_R16_UINT;
			srvDesc.Buffer.StructureByteStride = 0;
		}

		Descriptor->CreateShaderResourceView(m_DataBuffer->GetResource(), &srvDesc);
	}
	else
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
		destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		destTextureUAVDesc.Buffer.FirstElement = desc.FirstElement;
		destTextureUAVDesc.Buffer.NumElements = ElementCount;
		if (desc.Format == ETextureFormat::FORMAT_UNKNOWN)
		{
			destTextureUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
			destTextureUAVDesc.Buffer.StructureByteStride = ElementSize;
		}
		else
		{
			destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(desc.Format);
			destTextureUAVDesc.Buffer.StructureByteStride = 0;
		}
		destTextureUAVDesc.Buffer.CounterOffsetInBytes = CounterOffset;
		destTextureUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		Descriptor->CreateUnorderedAccessView(m_DataBuffer->GetResource(), nullptr, &destTextureUAVDesc);
	}
	Descriptor->Recreate();
	return Descriptor;
}



void D3D12Buffer::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype)
{
	BufferAccesstype = Accesstype;
#ifdef PLATFORM_ALWAYS_UMA
	BufferAccesstype = EBufferAccessType::Dynamic;
#endif
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

void D3D12Buffer::UpdateVertexBuffer(void* data, size_t length, int InVertexCount)
{
	VertexCount = length;
	if (InVertexCount != -1)
	{
		VertexCount = InVertexCount;
	}
	UpdateData(data, length, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	if (BufferAccesstype == EBufferAccessType::Dynamic)
	{
		PostUploadState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
}

void D3D12Buffer::SetBufferState(RHICommandList* list, EBufferResourceState::Type State)
{
	D3D12CommandList* d3dlist = D3D12RHI::DXConv(list);
	m_DataBuffer->SetResourceState(d3dlist, D3D12Helpers::ConvertBufferResourceState(State));
}

void D3D12Buffer::CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	List->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	ensure(List->GetDeviceIndex() == Context->GetDeviceIndex());
	D3D12InterGPUStagingResource* DXres = D3D12RHI::DXConv(Res);
	D3D12CommandList* list = D3D12RHI::DXConv(List);

	D3D12DeviceContext* CurrentDevice = D3D12RHI::DXConv(Context);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;
	GetResource()->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_SOURCE);
	DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->SetResourceState(list, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
	GPUResource* TargetResource = GetResource();

	list->FlushBarriers();
	list->GetCommandList()->CopyResource(DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->GetResource(), TargetResource->GetResource());

	GetResource()->SetResourceState(list, D3D12_RESOURCE_STATE_COMMON);
	list->FlushBarriers();
	List->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
}

void D3D12Buffer::CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	List->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	ensure(List->GetDeviceIndex() == Context->GetDeviceIndex());
	D3D12InterGPUStagingResource* DXres = D3D12RHI::DXConv(Res);
	D3D12CommandList* list = D3D12RHI::DXConv(List);

	D3D12DeviceContext* CurrentDevice = D3D12RHI::DXConv(Context);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;
	GetResource()->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
	DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->SetResourceState(list, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
	GPUResource* TargetResource = GetResource();

	list->FlushBarriers();
	list->GetCommandList()->CopyResource(TargetResource->GetResource(), DXres->GetViewOnDevice(CurrentDevice->GetDeviceIndex())->GetResource());

	GetResource()->SetResourceState(list, D3D12_RESOURCE_STATE_COMMON);
	list->FlushBarriers();
	List->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
}

void D3D12Buffer::UpdateData(void* data, size_t length, D3D12_RESOURCE_STATES EndState)
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
		UpdateBufferDataGPU(data, TotalByteSize, Device->GetCopyListDx());
	}
}

void D3D12Buffer::UpdateBufferDataGPU(void* data, size_t length, D3D12CommandList* list)
{
	AllocDesc D = AllocDesc();
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize);
	D.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	D.Segment = EGPUMemorysegment::Non_Local;
	if (Desc.UseForExecuteIndirect)
	{
		D.ResourceDesc.Flags |= D3D12RHIConfig::IndirectBufferResouceFlag;
	}
	Device->GetMemoryManager()->AllocResource(D, &m_UploadBuffer);
	D3D12Helpers::NameRHIObject(m_UploadBuffer, this, "(UPLOAD)");
	void* BufferData;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_UploadBuffer->GetResource()->Map(0, &readRange, &BufferData));
	memcpy(BufferData, data, length);
	m_UploadBuffer->GetResource()->Unmap(0, nullptr);
	if (list != nullptr)
	{
		PushDataUpLoad(list);
	}
}

void D3D12Buffer::PushDataUpLoad(D3D12CommandList * list)
{
	if (m_UploadBuffer != nullptr)
	{
		D3D12_RESOURCE_STATES BeforeState = m_DataBuffer->GetCurrentState();
		m_DataBuffer->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_DEST);
		list->GetCommandList()->CopyResource(m_DataBuffer->GetResource(), m_UploadBuffer->GetResource());
		m_DataBuffer->SetResourceState(list, BeforeState);
		RHI::AddToDeferredDeleteQueue(m_UploadBuffer);
		m_UploadBuffer = nullptr;
	}
}

bool D3D12Buffer::CheckDevice(int index)
{
	if (Device != nullptr)
	{
		return (Device->GetDeviceIndex() == index);
	}
	return false;
}

void D3D12Buffer::EnsureResouceInFinalState(D3D12CommandList* list)
{
	m_DataBuffer->SetResourceState(list, PostUploadState);
}

void D3D12Buffer::CreateStaticBuffer(int ByteSize)
{
	TotalByteSize = ByteSize;//max size

	AllocDesc D;
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);
	D.InitalState = D3D12_RESOURCE_STATE_COPY_DEST;
	Device->GetMemoryManager()->AllocResource(D, &m_DataBuffer);
}

void D3D12Buffer::CreateDynamicBuffer(int ByteSize)
{
	//This Buffer Will Have Data Changed Every frame so no need to transition to only gpu.
	// Create the vertex buffer.
	TotalByteSize = ByteSize;
	AllocDesc Allocdesc = AllocDesc(TotalByteSize, D3D12_RESOURCE_STATE_GENERIC_READ, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);
	Allocdesc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);
	Allocdesc.Segment = EGPUMemorysegment::Non_Local;
	if (Desc.UseForExecuteIndirect)
	{
		Allocdesc.ResourceDesc.Flags |= D3D12RHIConfig::IndirectBufferResouceFlag;
		Allocdesc.PageAllocationType = EPageTypes::BuffersOnly;
	}
	Allocdesc.IsReadBack = BufferAccesstype == EBufferAccessType::ReadBack;
	Device->GetMemoryManager()->AllocResource(Allocdesc, &m_DataBuffer);
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
		AllocDesc Allocdesc = {};
		Allocdesc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE);
		if (Desc.StartState == EResourceState::Undefined)
		{
			Allocdesc.InitalState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}
		else
		{
			Allocdesc.InitalState = D3D12Helpers::ConvertRHIState(Desc.StartState);
		}
		if (desc.UseForExecuteIndirect)
		{
			Allocdesc.ResourceDesc.Flags |= D3D12RHIConfig::IndirectBufferResouceFlag;
		}
		Device->GetMemoryManager()->AllocTemporaryGPU(Allocdesc, &m_DataBuffer);
	}
	else if (BufferAccesstype == EBufferAccessType::Dynamic)
	{
		CreateDynamicBuffer(TotalByteSize);
	}
	else if (BufferAccesstype == EBufferAccessType::Static)
	{
		CreateStaticBuffer(TotalByteSize);
	}
	else if (BufferAccesstype == EBufferAccessType::ReadBack)
	{
		CreateDynamicBuffer(TotalByteSize);
		PostUploadState = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	D3D12Helpers::NameRHIObject(m_DataBuffer, this);
}

void D3D12Buffer::UpdateIndexBuffer(void* data, size_t length)
{
	VertexCount = length;
	UpdateData(data, TotalByteSize, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void D3D12Buffer::UpdateBufferData(void* data, size_t length, EBufferResourceState::Type state)
{
	UpdateData(data, length, D3D12Helpers::ConvertBufferResourceState(state));
}

void D3D12Buffer::CreateIndexBuffer(int Stride, int ByteSize)
{

	TotalByteSize = ByteSize;
#ifdef PLATFORM_ALWAYS_UMA
	BufferAccesstype = EBufferAccessType::Dynamic;
	CreateDynamicBuffer(ByteSize);
#else
	CreateStaticBuffer(ByteSize);
#endif

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

GPUResource* D3D12Buffer::GetDoubleBuffer()
{
	return m_DataBufferDouble[Context->GetCpuFrameIndex()];
}

void D3D12Buffer::MapBuffer(void** Data)
{
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_DataBuffer->GetResource()->Map(0, &readRange, Data));
}

void D3D12Buffer::UnMap()
{
	m_DataBuffer->GetResource()->Unmap(0, nullptr);
}

void* D3D12Buffer::MapReadBack()
{
	void* Data = malloc(GetSize());
	CD3DX12_RANGE readRange(0, GetSize());		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_DataBuffer->GetResource()->Map(0, &readRange, &Data));

	return Data;
}

void D3D12Buffer::SetResourceState(RHICommandList* list, EResourceState::Type State)
{
	D3D12CommandList* d3dlist = D3D12RHI::DXConv(list);
	m_DataBuffer->SetResourceState(d3dlist, D3D12Helpers::ConvertRHIState(State));
}
