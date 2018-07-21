#include "stdafx.h"
#include "D3D12CommandList.h"
#include "D3D12RHI.h"
#include "Rendering/Core/RenderBaseTypes.h"
#include "Core/Utils/StringUtil.h"
#include "D3D12CBV.h"
#include "EngineGlobals.h"
#include "D3D12Texture.h"
#include "RHI/Shader.h"
#include "RHI/BaseTexture.h"
#include "D3D12Framebuffer.h"
#include "RHI/DeviceContext.h"
#include "DescriptorHeap.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Core/Utils/MemoryUtils.h"
D3D12CommandList::D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType) :RHICommandList(ListType)
{
	Device = inDevice;
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(Device->GetDevice()->CreateCommandAllocator(D3D12Helpers::ConvertListType(ListType), IID_PPV_ARGS(&m_commandAllocator[i])));
	}
	if (ListType == ECommandListType::Copy)
	{
		//copy queues don't have pipline states!
		CreateCommandList();
	}
}


D3D12CommandList::~D3D12CommandList()
{
	if (CurrentPipelinestate.m_pipelineState != nullptr)
	{
		CurrentPipelinestate.m_pipelineState->Release();
		CurrentPipelinestate.m_rootSignature->Release();
	}
	if (CurrentGraphicsList != nullptr)
	{
		CurrentGraphicsList->Release();
	}
}

void D3D12CommandList::ResetList()
{
	SCOPE_CYCLE_COUNTER_GROUP("ResetList", "RHI");
	ensure(!IsOpen);
	ThrowIfFailed(m_commandAllocator[Device->GetCpuFrameIndex()]->Reset());
	IsOpen = true;
	ThrowIfFailed(CurrentGraphicsList->Reset(m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState));
	if (ListType == ECommandListType::Graphics)
	{
		CurrentGraphicsList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
	}
	else if (ListType == ECommandListType::Compute)
	{
		CurrentGraphicsList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
	}
}

void D3D12CommandList::SetRenderTarget(FrameBuffer * target, int SubResourceIndex)
{
	ensure(ListType == ECommandListType::Graphics);
	if (target == nullptr)
	{
		CurrentRenderTarget->UnBind(CurrentGraphicsList);
		CurrentRenderTarget = nullptr;
	}
	else
	{
		CurrentRenderTarget = (D3D12FrameBuffer*)target;
		ensure(CurrentRenderTarget->CheckDevice(Device->GetDeviceIndex()));
		CurrentRenderTarget->BindBufferAsRenderTarget(CurrentGraphicsList, SubResourceIndex);
	}
}

void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	ensure(IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	if (Currentpipestate.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_LINE)
	{
		CurrentGraphicsList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	else if (Currentpipestate.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	{
		CurrentGraphicsList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	CurrentGraphicsList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{
	ensure(ListType == ECommandListType::Graphics);
}

void D3D12CommandList::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{
	//D3D12RHI::Instance->RenderToScreen(CurrentGraphicsList);
	ensure(ListType == ECommandListType::Graphics);
}

void D3D12CommandList::Execute(DeviceContextQueue::Type Target)
{
	if (Target == DeviceContextQueue::LIMIT)
	{
		switch (ListType)
		{
		case ECommandListType::Graphics:
			Target = DeviceContextQueue::Graphics;
			break;
		case ECommandListType::Compute:
			Target = DeviceContextQueue::Compute;
			break;
		case ECommandListType::Copy:
			Target = DeviceContextQueue::Copy;
			break;
		}
	}
	ThrowIfFailed(CurrentGraphicsList->Close());
	if (Target == DeviceContextQueue::Graphics)
	{
		Device->ExecuteCommandList(CurrentGraphicsList);
	}
	else if (Target == DeviceContextQueue::Compute)
	{
		Device->ExecuteComputeCommandList(CurrentGraphicsList);
	}
	else if (Target == DeviceContextQueue::Copy)
	{
		Device->ExecuteCopyCommandList(CurrentGraphicsList);
	}
	else if (Target == DeviceContextQueue::InterCopy)
	{
		Device->ExecuteInterGPUCopyCommandList(CurrentGraphicsList);
	}
	IsOpen = false;
}

void D3D12CommandList::WaitForCompletion()
{
	Device->EndExecuteCommandList();
}

void D3D12CommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	CurrentGraphicsList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
}

void D3D12CommandList::SetIndexBuffer(RHIBuffer * buffer)
{
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	CurrentGraphicsList->IASetIndexBuffer(&dbuffer->m_IndexBufferView);
}

void D3D12CommandList::CreatePipelineState(Shader * shader, class FrameBuffer* Buffer)
{
	ensure(ListType == ECommandListType::Graphics || ListType == ECommandListType::Compute);
	D3D12FrameBuffer* dbuffer = (D3D12FrameBuffer*)Buffer;
	D3D12Shader::PipeRenderTargetDesc PRTD = {};
	if (Buffer == nullptr)
	{
		PRTD.NumRenderTargets = 1;
		PRTD.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		PRTD.DSVFormat = DXGI_FORMAT_D32_FLOAT;//DXGI_FORMAT_D16_UNORM
	}
	else
	{
		PRTD = dbuffer->GetPiplineRenderDesc();
	}
	CreatePipelineState(shader, PRTD);
}

void D3D12CommandList::CreatePipelineState(Shader * shader, D3D12Shader::PipeRenderTargetDesc RTdesc)
{
	if (shader->IsComputeShader())
	{
		ensure(ListType == ECommandListType::Compute);
	}
	else
	{
		ensure(ListType == ECommandListType::Graphics);
	}
	if (CurrentPipelinestate.m_pipelineState != nullptr)
	{
		CurrentPipelinestate.m_pipelineState->Release();
	}
	if (CurrentPipelinestate.m_rootSignature != nullptr)
	{
		CurrentPipelinestate.m_rootSignature->Release();
	}
	CurrentPipelinestate.IsCompute = (ListType == ECommandListType::Compute);
	D3D12Shader* target = (D3D12Shader*)shader->GetShaderProgram();
	ensure(target != nullptr);
	ensure((shader->GetShaderParameters().size() > 0));
	ensure((shader->GetVertexFormat().size() > 0));
	D3D12_INPUT_ELEMENT_DESC* desc;
	D3D12Shader::ParseVertexFormat(shader->GetVertexFormat(), &desc, &VertexDesc_ElementCount);
	D3D12Shader::CreateRootSig(CurrentPipelinestate, shader->GetShaderParameters(), Device);

	if (ListType == ECommandListType::Graphics)
	{
		D3D12Shader::CreatePipelineShader(CurrentPipelinestate, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Currentpipestate, RTdesc, Device);
	}
	else
	{
		D3D12Shader::CreateComputePipelineShader(CurrentPipelinestate, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Currentpipestate, RTdesc, Device);
	}
	if (CurrentGraphicsList == nullptr)
	{
		//todo: ensure a gaphics shader is not used a compute piplne!
		CreateCommandList();
	}
}

void D3D12CommandList::SetPipelineState(PipeLineState state)
{
	Currentpipestate = state;
}

void D3D12CommandList::CreateCommandList()
{
	if (ListType == ECommandListType::Graphics)
	{
		ThrowIfFailed(Device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentGraphicsList)));
		CurrentGraphicsList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
		ThrowIfFailed(CurrentGraphicsList->Close());
	}
	else if (ListType == ECommandListType::Compute)
	{
		//todo: aloccators?
		ThrowIfFailed(Device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentGraphicsList)));
		CurrentGraphicsList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
		ThrowIfFailed(CurrentGraphicsList->Close());
	}
	else if (ListType == ECommandListType::Copy)
	{
		ThrowIfFailed(Device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentGraphicsList)));
		//CurrentGraphicsList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
		ThrowIfFailed(CurrentGraphicsList->Close());
	}
}

void D3D12CommandList::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{
	ensure(ListType == ECommandListType::Compute);
	CurrentGraphicsList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void D3D12CommandList::CopyResourceToSharedMemory(FrameBuffer * Buffer)
{
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)Buffer;
	ensure(Device == buffer->GetDevice());
	buffer->CopyToDevice(CurrentGraphicsList);
}

void D3D12CommandList::CopyResourceFromSharedMemory(FrameBuffer * Buffer)
{
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)Buffer;
	ensure(Device == buffer->GetTargetDevice());
	buffer->MakeReadyOnTarget(CurrentGraphicsList);
}

void D3D12CommandList::ClearFrameBuffer(FrameBuffer * buffer)
{
	ensure(ListType == ECommandListType::Graphics);
	((D3D12FrameBuffer*)buffer)->ClearBuffer(CurrentGraphicsList);
}

void D3D12CommandList::UAVBarrier(RHIUAV * target)
{
	D3D12RHIUAV* dtarget = (D3D12RHIUAV*)target;//todo: counter uav?
	CurrentGraphicsList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->UAVCounter));
}

void D3D12CommandList::SetScreenBackBufferAsRT()
{
	ensure(ListType == ECommandListType::Graphics);
	if (CurrentRenderTarget != nullptr)
	{
		CurrentRenderTarget->UnBind(CurrentGraphicsList);
		CurrentRenderTarget = nullptr;
	}
	ensureMsgf(Device->GetDeviceIndex() == 0, "Only the Primary Device Is allowed to write to the backbuffer");
	D3D12RHI::Instance->SetScreenRenderTarget(CurrentGraphicsList);
	D3D12RHI::Instance->RenderToScreen(CurrentGraphicsList);
}

void D3D12CommandList::ClearScreen()
{
	ensure(ListType == ECommandListType::Graphics);
	ensureMsgf(Device->GetDeviceIndex() == 0, "Only the Primary Device Is allowed to write to the backbuffer");
	D3D12RHI::Instance->ClearRenderTarget(CurrentGraphicsList);
}

void D3D12CommandList::SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex)
{
	ensure(ListType == ECommandListType::Graphics || ListType == ECommandListType::Compute);
	D3D12FrameBuffer* DBuffer = (D3D12FrameBuffer*)buffer;
#if 0
	if (buffer == nullptr)
	{
		if (GPUStateCache::instance->TextureBuffers[slot] != nullptr)
		{
			((D3D12FrameBuffer*)GPUStateCache::instance->TextureBuffers[slot])->UnBind(CurrentGraphicsList);
			GPUStateCache::instance->TextureBuffers[slot] = nullptr;
		}
		return;
	}
	else
	{
		GPUStateCache::instance->TextureBuffers[slot] = DBuffer;
	}
#endif
	ensure(DBuffer->CheckDevice(Device->GetDeviceIndex()));
	DBuffer->BindBufferToTexture(CurrentGraphicsList, slot, Resourceindex, Device, (ListType == ECommandListType::Compute));
}

void D3D12CommandList::SetTexture(BaseTexture * texture, int slot)
{
	Texture = (D3D12Texture*)texture;
	ensureMsgf(Texture->CheckDevice(Device->GetDeviceIndex()), "Attempted to Bind texture that is not on this device");
	if (CurrentGraphicsList != nullptr)
	{
		Texture->BindToSlot(CurrentGraphicsList, slot);
	}
}

void D3D12CommandList::UpdateConstantBuffer(void * data, int offset)
{
	CurrentConstantBuffer->UpdateConstantBuffer(data, offset);
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot)
{
	D3D12Buffer* d3Buffer = (D3D12Buffer*)buffer;
	ensure(d3Buffer->CheckDevice(Device->GetDeviceIndex()));
	d3Buffer->SetConstantBufferView(offset, CurrentGraphicsList, Slot, ListType == ECommandListType::Compute,Device->GetDeviceIndex());
}


D3D12Buffer::D3D12Buffer(RHIBuffer::BufferType type, DeviceContext * inDevice) :RHIBuffer(type)
{
	if (inDevice == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	else
	{
		Device = inDevice;
	}
}

D3D12Buffer::~D3D12Buffer()
{
	Device = nullptr;
	if (CurrentBufferType == RHIBuffer::BufferType::Constant)
	{
		MemoryUtils::DeleteCArray(CBV, MAX_DEVICE_COUNT);
	}
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
	}
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
	}
}

void D3D12Buffer::CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices)
{
	ensure(StructSize > 0);
	ensure(Elementcount > 0);
	ConstantBufferDataSize = StructSize;
	CrossDevice = ReplicateToAllDevices;
	if (ReplicateToAllDevices)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			CBV[i] = new D3D12CBV(RHI::GetDeviceContext(i));
			CBV[i]->InitCBV(StructSize, Elementcount);
		}
	}
	else
	{
		CBV[0] = new D3D12CBV(Device);
		CBV[0]->InitCBV(StructSize, Elementcount);
	}
}
void D3D12Buffer::UpdateConstantBuffer(void * data, int offset)
{
	if (CrossDevice)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			CBV[i]->UpdateCBV(data, offset, ConstantBufferDataSize);
		}
	}
	else
	{
		CBV[0]->UpdateCBV(data, offset, ConstantBufferDataSize);
	}
}

void D3D12Buffer::SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Slot, bool  IsCompute,int Deviceindex)
{
	if (CrossDevice)
	{
		CBV[Deviceindex]->SetDescriptorHeaps(list);//D3D12CBV::MPCBV
		CBV[Deviceindex]->SetGpuView(list, offset, Slot, IsCompute);//todo: handle Offset!
	}
	else
	{
		CBV[0]->SetDescriptorHeaps(list);//D3D12CBV::MPCBV
		CBV[0]->SetGpuView(list, offset, Slot, IsCompute);//todo: handle Offset!
	}
}

void D3D12Buffer::CreateVertexBuffer(int Stride, int ByteSize, BufferAccessType Accesstype)
{
	BufferAccesstype = Accesstype;
	if (BufferAccesstype == BufferAccessType::Dynamic)
	{
		CreateDynamicBuffer(Stride, ByteSize);
	}
	else if (BufferAccesstype == BufferAccessType::Static)
	{
		CreateStaticBuffer(Stride, ByteSize);
	}
}

void D3D12Buffer::UpdateVertexBuffer(void * data, size_t length)
{
	VertexCount = length;
	if (BufferAccesstype == BufferAccessType::Dynamic)
	{
		UINT8* pVertexDataBegin;
		MapBuffer(reinterpret_cast<void**>(&pVertexDataBegin));

		memcpy(pVertexDataBegin, data, length);
		UnMap();
	}
	else
	{
		ensure(!UploadComplete && "Uploading More than once to a GPU only buffer is not allowed!");
		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA Data = {};
		Data.pData = reinterpret_cast<BYTE*>(data); // pointer to our index array
		Data.RowPitch = VertexBufferSize; // size of all our index buffer
		Data.SlicePitch = VertexBufferSize; // also the size of our index buffer

											// we are now creating a command with the command list to copy the data from
											// the upload heap to the default heap
		UpdateSubresources(Device->GetCopyList(), m_vertexBuffer, m_UploadBuffer, 0, 0, 1, &Data);

		// transition the vertex buffer data from copy destination state to vertex buffer state
		Device->GetCopyList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		UploadComplete = true;
		Device->NotifyWorkForCopyEngine();
		D3D12RHI::Instance->AddUploadToUsed(m_UploadBuffer);
	}
}

bool D3D12Buffer::CheckDevice(int index)
{
	if (CurrentBufferType == RHIBuffer::BufferType::Constant && CrossDevice)
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

void D3D12Buffer::CreateStaticBuffer(int Stride, int ByteSize)
{
	const int vertexBufferSize = ByteSize;//mazsize
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&m_vertexBuffer)));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	m_vertexBuffer->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)));
	m_UploadBuffer->SetName(L"Index Buffer Upload Resource Heap");
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = Stride;
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBuffer->SetName(StringUtils::ConvertStringToWide("Vertex Buffer").c_str());
}

void D3D12Buffer::CreateDynamicBuffer(int Stride, int ByteSize)
{
	//This Vertex Buffer Will Have Data Changed Every frame so no need to transiton to only gpu.
	// Create the vertex buffer.
	VertexBufferSize = ByteSize;//mazsize

	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = Stride;
	m_vertexBufferView.SizeInBytes = VertexBufferSize;
	m_vertexBuffer->SetName(StringUtils::ConvertStringToWide("Vertex Buffer").c_str());
}




void D3D12Buffer::UpdateIndexBuffer(void * data, size_t length)
{
	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, data, length);
	m_indexBuffer->Unmap(0, nullptr);
}

void D3D12Buffer::CreateIndexBuffer(int Stride, int ByteSize)
{
	const int vertexBufferSize = ByteSize;

	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)));

	// Initialize the vertex buffer view.
	m_IndexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = vertexBufferSize;
	m_indexBuffer->SetName(StringUtils::ConvertStringToWide("Vertex Buffer").c_str());
}

void D3D12Buffer::MapBuffer(void ** Data)
{
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, Data));
}

void D3D12Buffer::UnMap()
{
	m_vertexBuffer->Unmap(0, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//UAV 
D3D12RHIUAV::D3D12RHIUAV(DeviceContext * inDevice) : RHIUAV()
{
	Device = inDevice;
}

D3D12RHIUAV::~D3D12RHIUAV()
{
	Heap->Release();
}

void D3D12RHIUAV::CreateUAVFromTexture(BaseTexture * target)
{
	Heap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	destTextureUAVDesc.Format = ((D3D12Texture*)target)->GetResource()->GetDesc().Format;
	destTextureUAVDesc.Texture2D.MipSlice = 1;
	//todo:Counter UAV?
	Device->GetDevice()->CreateUnorderedAccessView(((D3D12Texture*)target)->GetResource(), UAVCounter, &destTextureUAVDesc, Heap->GetCPUAddress(0));
}

void D3D12RHIUAV::CreateUAVFromFrameBuffer(FrameBuffer * target)
{
	Heap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(target->GetDescription().RTFormats[0]);
	destTextureUAVDesc.Texture2D.MipSlice = 0;
	//todo:Counter UAV?
	Device->GetDevice()->CreateUnorderedAccessView(((D3D12FrameBuffer*)target)->GetResource(0)->GetResource(), UAVCounter, &destTextureUAVDesc, Heap->GetCPUAddress(0));
}

void D3D12RHIUAV::Bind(RHICommandList * list, int slot)
{
	D3D12CommandList* DXList = ((D3D12CommandList*)list);
	Heap->BindHeap(DXList->GetCommandList());
	DXList->GetCommandList()->SetComputeRootDescriptorTable(slot, Heap->GetGpuAddress(0));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Texture Array
D3D12RHITextureArray::D3D12RHITextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
{
	Heap = new DescriptorHeap(device, NumEntries, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Device = device;
}

D3D12RHITextureArray::~D3D12RHITextureArray()
{
	delete Heap;
}

//Add a framebuffer to this heap and ask it to create one in our heap
void D3D12RHITextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{
	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)Buffer;
	LinkedBuffers.push_back(dBuffer);
	dBuffer->CreateSRVInHeap(slot, Heap);
	NullHeapDesc = dBuffer->GetSrvDesc(0);
}

void D3D12RHITextureArray::BindToShader(RHICommandList * list, int slot)
{
	D3D12CommandList* DXList = ((D3D12CommandList*)list);
	for (int i = 0; i < LinkedBuffers.size(); i++)
	{
		LinkedBuffers[i]->ReadyResourcesForRead(DXList->GetCommandList());
	}
	Heap->BindHeap(DXList->GetCommandList());
	DXList->GetCommandList()->SetGraphicsRootDescriptorTable(slot, Heap->GetGpuAddress(0));
}

//Makes a descriptor Null Using the first framebuffers Description
void D3D12RHITextureArray::SetIndexNull(int TargetIndex)
{
	Device->GetDevice()->CreateShaderResourceView(nullptr, &NullHeapDesc, Heap->GetCPUAddress(TargetIndex));
}
