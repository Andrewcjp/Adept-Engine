#include "stdafx.h"
#include "D3D12CommandList.h"
#include "D3D12RHI.h"
#include "Rendering/Core/RenderBaseTypes.h"
#include "Core/Utils/StringUtil.h"
#include "D3D12CBV.h"

#include "D3D12Texture.h"
#include "RHI/RHI_inc.h"
#include "D3D12Framebuffer.h"
#include "RHI/DeviceContext.h"
#include "DescriptorHeap.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Core/Utils/MemoryUtils.h"
#include "D3D12DeviceContext.h"
#include "Core/Platform/PlatformCore.h"

D3D12CommandList::D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType) :RHICommandList(ListType)
{
	AddCheckerRef(D3D12CommandList, this);
	Device = inDevice;
	mDeviceContext = (D3D12DeviceContext*)inDevice;
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandAllocator(D3D12Helpers::ConvertListType(ListType), IID_PPV_ARGS(&m_commandAllocator[i])));
	}
	if (ListType == ECommandListType::Copy)
	{
		//copy queues don't have pipline states!
		CreateCommandList();
	}
}

void D3D12CommandList::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12CommandList, this);
	MemoryUtils::DeleteReleaseableMap<std::string, D3D12PiplineShader>(PSOCache);
	SafeRelease(CurrentCommandList);
	MemoryUtils::DeleteReleaseableCArray(m_commandAllocator, RHI::CPUFrameCount);
	if (PSOCache.size() == 0)
	{
		CurrentPipelinestate.Release();
	}
}

D3D12CommandList::~D3D12CommandList()
{}

void D3D12CommandList::ResetList()
{
	SCOPE_CYCLE_COUNTER_GROUP("ResetList", "RHI");
	ensure(!m_IsOpen);
	ThrowIfFailed(m_commandAllocator[Device->GetCpuFrameIndex()]->Reset());
	m_IsOpen = true;
	ensure(CurrentCommandList != nullptr);
	ThrowIfFailed(CurrentCommandList->Reset(m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState));
	if (ListType == ECommandListType::Graphics)
	{
		CurrentCommandList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
	}
	else if (ListType == ECommandListType::Compute)
	{
		CurrentCommandList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
	}
}

void D3D12CommandList::SetRenderTarget(FrameBuffer * target, int SubResourceIndex)
{
	ensure(ListType == ECommandListType::Graphics);
	if (target == nullptr)
	{
		if (CurrentRenderTarget != nullptr)
		{
			CurrentRenderTarget->UnBind(CurrentCommandList);
			CurrentRenderTarget = nullptr;
		}
	}
	else
	{
		ensure(!target->IsPendingKill());
		CurrentRenderTarget = (D3D12FrameBuffer*)target;
		ensure(CurrentRenderTarget->CheckDevice(Device->GetDeviceIndex()));
		CurrentRenderTarget->BindBufferAsRenderTarget(CurrentCommandList, SubResourceIndex);
	}
}

void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	ensure(m_IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	if (Currentpipestate.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_LINE)
	{
		CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	else if (Currentpipestate.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	{
		CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	CurrentCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
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
	SCOPE_CYCLE_COUNTER_GROUP("Execute", "RHI");
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
	ThrowIfFailed(CurrentCommandList->Close());
	if (Target == DeviceContextQueue::Graphics)
	{
		mDeviceContext->ExecuteCommandList(CurrentCommandList);
	}
	else if (Target == DeviceContextQueue::Compute)
	{
		mDeviceContext->ExecuteComputeCommandList(CurrentCommandList);
	}
	else if (Target == DeviceContextQueue::Copy)
	{
		mDeviceContext->ExecuteCopyCommandList(CurrentCommandList);
	}
	else if (Target == DeviceContextQueue::InterCopy)
	{
		mDeviceContext->ExecuteInterGPUCopyCommandList(CurrentCommandList);
	}
	m_IsOpen = false;
}

void D3D12CommandList::WaitForCompletion()
{
	mDeviceContext->EndExecuteCommandList();
}

void D3D12CommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	CurrentCommandList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
}

void D3D12CommandList::SetIndexBuffer(RHIBuffer * buffer)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	CurrentCommandList->IASetIndexBuffer(&dbuffer->m_IndexBufferView);
}

void D3D12CommandList::SetPipelineState(PipeLineState state)
{
	Currentpipestate = state;
}

void D3D12CommandList::CreatePipelineState(Shader * shader, class FrameBuffer* Buffer)
{
	ensure(ListType == ECommandListType::Graphics || ListType == ECommandListType::Compute);
	if (Buffer != nullptr)
	{
		ensure(!Buffer->IsPendingKill());
		Currentpipestate.RenderTargetDesc = Buffer->GetPiplineRenderDesc();
	}
	if (Currentpipestate.RenderTargetDesc.NumRenderTargets == 0 && Currentpipestate.RenderTargetDesc.DSVFormat == eTEXTURE_FORMAT::FORMAT_UNKNOWN)
	{
		Currentpipestate.RenderTargetDesc.NumRenderTargets = 1;
		Currentpipestate.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
		Currentpipestate.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	}
	IN_CreatePipelineState(shader);
}

void D3D12CommandList::SetPipelineStateObject(Shader * shader, FrameBuffer * Buffer)
{
	if (Buffer != nullptr)
	{
		ensure(!Buffer->IsPendingKill());
	}
	bool IsChanged = false;
	if (PSOCache.find(shader->GetName()) != PSOCache.end())
	{
		if (PSOCache.at(shader->GetName()) != CurrentPipelinestate)
		{
			CurrentPipelinestate = PSOCache.at(shader->GetName());
			IsChanged = true;
		}
	}
	else
	{
		CreatePipelineState(shader, Buffer);
		IsChanged = true;
		if (RHI::GetFrameCount() > 2)
		{
			Log::LogMessage("Created a PSO at runtime", Log::Severity::Warning);
		}
	}
	if (IsChanged && IsOpen())
	{
		CurrentCommandList->SetPipelineState(CurrentPipelinestate.m_pipelineState);
		CurrentCommandList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
	}
}

void D3D12CommandList::IN_CreatePipelineState(Shader * shader)
{
	if (shader->IsComputeShader())
	{
		ensure(ListType == ECommandListType::Compute);
	}
	else
	{
		ensure(ListType == ECommandListType::Graphics);
	}

	CurrentPipelinestate.IsCompute = (ListType == ECommandListType::Compute);
	D3D12Shader* target = (D3D12Shader*)shader->GetShaderProgram();
	ensure(target != nullptr);
	ensure((shader->GetShaderParameters().size() > 0));
	ensure((shader->GetVertexFormat().size() > 0));
	D3D12_INPUT_ELEMENT_DESC* desc;
	D3D12Shader::ParseVertexFormat(shader->GetVertexFormat(), &desc, &VertexDesc_ElementCount);
	D3D12Shader::CreateRootSig(CurrentPipelinestate, shader->GetShaderParameters(), Device);

	VertexDesc = *desc;
	Params = shader->GetShaderParameters();

	if (ListType == ECommandListType::Graphics)
	{
		D3D12Shader::CreatePipelineShader(CurrentPipelinestate, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Currentpipestate, Device);
	}
	else if (ListType == ECommandListType::Compute)
	{
		D3D12Shader::CreateComputePipelineShader(CurrentPipelinestate, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Currentpipestate, Device);
	}
	if (CurrentCommandList == nullptr)
	{
		CreateCommandList();
	}
	PSOCache.try_emplace(shader->GetName(), CurrentPipelinestate);
}

void D3D12CommandList::CreateCommandList()
{
	if (ListType == ECommandListType::Graphics)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentCommandList)));
		CurrentCommandList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
		if (CurrentPipelinestate.m_pipelineState != nullptr)
		{
			CurrentCommandList->SetPipelineState(CurrentPipelinestate.m_pipelineState);
		}
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Compute)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentCommandList)));
		CurrentCommandList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
		CurrentCommandList->SetPipelineState(CurrentPipelinestate.m_pipelineState);
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Copy)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_commandAllocator[Device->GetCpuFrameIndex()], CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
}

void D3D12CommandList::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{
	ensure(ListType == ECommandListType::Compute);
	CurrentCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void D3D12CommandList::CopyResourceToSharedMemory(FrameBuffer * Buffer)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)Buffer;
	ensure(Device == buffer->GetDevice());
	buffer->CopyToDevice(CurrentCommandList);
}

void D3D12CommandList::CopyResourceFromSharedMemory(FrameBuffer * Buffer)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)Buffer;
	ensure(Device == buffer->GetTargetDevice());
	buffer->MakeReadyOnTarget(CurrentCommandList);
}

void D3D12CommandList::ClearFrameBuffer(FrameBuffer * buffer)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	((D3D12FrameBuffer*)buffer)->ClearBuffer(CurrentCommandList);
}

void D3D12CommandList::UAVBarrier(RHIUAV * target)
{
	ensure(!target->IsPendingKill());
	D3D12RHIUAV* dtarget = (D3D12RHIUAV*)target;//todo: counter uav?
	CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->UAVCounter));
}

ID3D12GraphicsCommandList * D3D12CommandList::GetCommandList()
{
	ensure(m_IsOpen);
	return CurrentCommandList;
}

void D3D12CommandList::SetScreenBackBufferAsRT()
{
	ensure(ListType == ECommandListType::Graphics);
	if (CurrentRenderTarget != nullptr)
	{
		CurrentRenderTarget->UnBind(CurrentCommandList);
		CurrentRenderTarget = nullptr;
	}
	ensureMsgf(Device->GetDeviceIndex() == 0, "Only the Primary Device Is allowed to write to the backbuffer");
	D3D12RHI::Instance->SetScreenRenderTarget(CurrentCommandList);
	D3D12RHI::Instance->RenderToScreen(CurrentCommandList);
}

void D3D12CommandList::ClearScreen()
{
	ensure(ListType == ECommandListType::Graphics);
	ensureMsgf(Device->GetDeviceIndex() == 0, "Only the Primary Device Is allowed to write to the backbuffer");
	D3D12RHI::Instance->ClearRenderTarget(CurrentCommandList);
}

void D3D12CommandList::SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics || ListType == ECommandListType::Compute);
	D3D12FrameBuffer* DBuffer = (D3D12FrameBuffer*)buffer;
#if 0
	if (buffer == nullptr)
	{
		if (GPUStateCache::instance->TextureBuffers[slot] != nullptr)
		{
			((D3D12FrameBuffer*)GPUStateCache::instance->TextureBuffers[slot])->UnBind(CurrentCommandList);
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
	DBuffer->BindBufferToTexture(CurrentCommandList, slot, Resourceindex, Device, (ListType == ECommandListType::Compute));
}

void D3D12CommandList::SetTexture(BaseTexture * texture, int slot)
{
	Texture = (D3D12Texture*)texture;
	ensure(texture);
	ensure(!texture->IsPendingKill());
	ensureMsgf(Texture->CheckDevice(Device->GetDeviceIndex()), "Attempted to Bind texture that is not on this device");
	if (CurrentCommandList != nullptr)
	{
		Texture->BindToSlot(CurrentCommandList, slot);
	}
}

void D3D12CommandList::UpdateConstantBuffer(void * data, int offset)
{
	CurrentConstantBuffer->UpdateConstantBuffer(data, offset);
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot)
{
	ensure(!buffer->IsPendingKill());
	D3D12Buffer* d3Buffer = (D3D12Buffer*)buffer;
	ensure(d3Buffer->CheckDevice(Device->GetDeviceIndex()));
	d3Buffer->SetConstantBufferView(offset, CurrentCommandList, Slot, ListType == ECommandListType::Compute, Device->GetDeviceIndex());
}


D3D12Buffer::D3D12Buffer(RHIBuffer::BufferType type, DeviceContext * inDevice) :RHIBuffer(type)
{
	AddCheckerRef(D3D12Buffer, this);
	if (inDevice == nullptr)
	{
		Device = (D3D12DeviceContext*)RHI::GetDefaultDevice();
	}
	else
	{
		Device = (D3D12DeviceContext*)inDevice;
	}
}
void D3D12Buffer::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12Buffer, this);
	Device = nullptr;
	if (CurrentBufferType == RHIBuffer::BufferType::Constant)
	{
		MemoryUtils::DeleteCArray(CBV, MAX_DEVICE_COUNT);
	}
	SafeRelease(m_vertexBuffer);
	SafeRelease(m_indexBuffer);
}

D3D12Buffer::~D3D12Buffer()
{}

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

void D3D12Buffer::SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Slot, bool  IsCompute, int Deviceindex)
{
	if (CrossDevice)
	{
		CBV[Deviceindex]->SetDescriptorHeaps(list);
		CBV[Deviceindex]->SetGpuView(list, offset, Slot, IsCompute);
	}
	else
	{
		CBV[0]->SetDescriptorHeaps(list);
		CBV[0]->SetGpuView(list, offset, Slot, IsCompute);
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
		D3D12RHI::Instance->AddObjectToDeferredDeleteQueue(m_UploadBuffer);
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
	D3D12RHI::Instance->AddObjectToDeferredDeleteQueue(m_UploadBuffer);
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
	Device = (D3D12DeviceContext*)inDevice;
	AddCheckerRef(D3D12RHIUAV, this);
}
void D3D12RHIUAV::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12RHIUAV, this);
	Heap->Release();
	SafeRelease(UAVCounter);
	SafeRelease(m_UAV);
}
D3D12RHIUAV::~D3D12RHIUAV()
{}

void D3D12RHIUAV::CreateUAVFromTexture(BaseTexture * target)
{
	D3D12Texture* D3DTarget = (D3D12Texture*)target;
	ensure(D3DTarget->CheckDevice(Device->GetDeviceIndex()));
	Heap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	Heap->SetName(L"CreateUAVFromTexture");
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	destTextureUAVDesc.Format = ((D3D12Texture*)target)->GetResource()->GetDesc().Format;
	destTextureUAVDesc.Texture2D.MipSlice = 1;
	//todo:Counter UAV?
	Device->GetDevice()->CreateUnorderedAccessView(((D3D12Texture*)target)->GetResource(), UAVCounter, &destTextureUAVDesc, Heap->GetCPUAddress(0));
}

void D3D12RHIUAV::CreateUAVFromFrameBuffer(FrameBuffer * target)
{
	D3D12FrameBuffer* D3DTarget = (D3D12FrameBuffer*)target;
	ensure(D3DTarget->CheckDevice(Device->GetDeviceIndex()));
	Heap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	Heap->SetName(L"CreateUAVFromFrameBuffer");
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(target->GetDescription().RTFormats[0]);
	destTextureUAVDesc.Texture2D.MipSlice = 0;
	//todo:Counter UAV?
	Device->GetDevice()->CreateUnorderedAccessView(((D3D12FrameBuffer*)target)->GetResource(0)->GetResource(), UAVCounter, &destTextureUAVDesc, Heap->GetCPUAddress(0));
}

void D3D12RHIUAV::Bind(RHICommandList * list, int slot)
{
	ensure(Device == list->GetDevice());
	D3D12CommandList* DXList = ((D3D12CommandList*)list);
	Heap->BindHeap(DXList->GetCommandList());
	DXList->GetCommandList()->SetComputeRootDescriptorTable(slot, Heap->GetGpuAddress(0));
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Texture Array
D3D12RHITextureArray::D3D12RHITextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
{
	AddCheckerRef(D3D12RHITextureArray, this);
	Heap = new DescriptorHeap(device, NumEntries, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Heap->SetName(L"D3D12RHITextureArray");
	Device = (D3D12DeviceContext*)device;
}

D3D12RHITextureArray::~D3D12RHITextureArray()
{}

//Add a framebuffer to this heap and ask it to create one in our heap
void D3D12RHITextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)Buffer;
	ensure(dBuffer->CheckDevice(Device->GetDeviceIndex()));
	LinkedBuffers.push_back(dBuffer);
	dBuffer->CreateSRVInHeap(slot, Heap);
	NullHeapDesc = dBuffer->GetSrvDesc(0);
}

void D3D12RHITextureArray::BindToShader(RHICommandList * list, int slot)
{
	D3D12CommandList* DXList = ((D3D12CommandList*)list);
	ensure(DXList->GetDevice() == Device);
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

void D3D12RHITextureArray::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12RHITextureArray, this);
	SafeDelete(Heap);
}
