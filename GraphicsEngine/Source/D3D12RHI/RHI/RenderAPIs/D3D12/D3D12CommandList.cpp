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
#include "DescriptorHeap.h"
D3D12CommandList::D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType) :RHICommandList(ListType, inDevice)
{
	AddCheckerRef(D3D12CommandList, this);
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
	SafeRelease(CommandSig);
}

void D3D12CommandList::ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{
	ensure(CommandSig != nullptr);
	ensure(ArgumentBuffer);
	ID3D12Resource* counterB = nullptr;
	if (CountBuffer != nullptr)
	{
		counterB = ((D3D12Buffer*)CountBuffer)->GetResource()->GetResource();
	}
	//PushPrimitiveTopology();
	CurrentCommandList->ExecuteIndirect(CommandSig, MaxCommandCount, ((D3D12Buffer*)ArgumentBuffer)->GetResource()->GetResource(), ArgOffset, counterB, CountBufferOffset);
}

void D3D12CommandList::PushPrimitiveTopology()
{
	if (IsGraphicsList())
	{
		if (Currentpipestate.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_LINE)
		{
			CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		}
		else if (Currentpipestate.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		{
			CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
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
	PushPrimitiveTopology();
	CurrentCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{
	ensure(ListType == ECommandListType::Graphics);
}

void D3D12CommandList::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{
	ensure(ListType == ECommandListType::Graphics);
	CD3DX12_VIEWPORT m_viewport = CD3DX12_VIEWPORT((FLOAT)MinX, (FLOAT)MinY, (FLOAT)MaxX, (FLOAT)MaxY);
	CurrentCommandList->RSSetViewports(1, &m_viewport);
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

void D3D12CommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	dbuffer->EnsureResouceInFinalState(GetCommandList());
	CurrentCommandList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
	PushPrimitiveTopology();
}

void D3D12CommandList::SetIndexBuffer(RHIBuffer * buffer)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	dbuffer->EnsureResouceInFinalState(GetCommandList());
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

std::string D3D12CommandList::GetPSOHash(Shader* shader,const RHIPipeRenderTargetDesc & statedesc)
{
	std::string hash = "";
	hash += shader->GetName();	
	hash += std::to_string((int)statedesc.RTVFormats[0]);	
	return hash;
}

void D3D12CommandList::SetPipelineStateObject(Shader * shader, FrameBuffer * Buffer)
{
	if (Buffer != nullptr)
	{
		ensure(!Buffer->IsPendingKill());
	}
	bool IsChanged = false;
	std::string Hash = GetPSOHash(shader, Currentpipestate.RenderTargetDesc);
	if (Buffer != nullptr)
	{
		Hash = GetPSOHash(shader, Buffer->GetPiplineRenderDesc());
	}
	if (PSOCache.find(Hash) != PSOCache.end())
	{
		if (PSOCache.at(Hash) != CurrentPipelinestate)
		{
			CurrentPipelinestate = PSOCache.at(Hash);
			CurrnetPsoKey = Hash;
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
		if (IsGraphicsList())
		{
			CurrentCommandList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
		}
		else
		{
			CurrentCommandList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
		}
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
	const std::string Hash = GetPSOHash(shader, Currentpipestate.RenderTargetDesc);
	
	PSOCache.try_emplace(Hash, CurrentPipelinestate);
}

void D3D12CommandList::CreateCommandList()
{
	if (CurrentCommandList != nullptr)
	{
		return;
	}
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
	D3D12Helpers::NameRHIObject(CurrentCommandList, this);
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

void D3D12CommandList::SetUpCommandSigniture(int commandSize, bool Dispatch)
{
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	if (Dispatch)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		ensure(commandSize % 4 == 0);

		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = commandSize;
	}
	else
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[0].Constant.RootParameterIndex = 0;
		argumentDescs[0].Constant.Num32BitValuesToSet = 1;
		argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
		argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
		ensure(commandSize % 4 == 0);

		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = commandSize;
	}


	ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandSignature(&commandSignatureDesc, Dispatch ? nullptr : CurrentPipelinestate.m_rootSignature, IID_PPV_ARGS(&CommandSig)));
	NAME_D3D12_OBJECT(CommandSig);

}

void D3D12CommandList::SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset)
{
	if (IsComputeList())
	{
		CurrentCommandList->SetComputeRoot32BitConstants(SignitureSlot, ValueNum, Data, DataOffset);
	}
	else
	{
		CurrentCommandList->SetGraphicsRoot32BitConstants(SignitureSlot, ValueNum, Data, DataOffset);
	}

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


D3D12Buffer::D3D12Buffer(ERHIBufferType::Type type, DeviceContext * inDevice) :RHIBuffer(type)
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
	if (CurrentBufferType == ERHIBufferType::Constant)
	{
		MemoryUtils::DeleteCArray(CBV, MAX_GPU_DEVICE_COUNT);
	}
	SafeRelease(m_DataBuffer);
	SafeRelease(SRVBufferHeap);
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
			CBV[i] = new D3D12CBV(RHI::GetDeviceContext(i));
			CBV[i]->InitCBV(StructSize, Elementcount);
			D3D12Helpers::NameRHIObject(CBV[i], this);
		}
	}
	else
	{
		CBV[0] = new D3D12CBV(Device);
		CBV[0]->InitCBV(StructSize, Elementcount);
		D3D12Helpers::NameRHIObject(CBV[0], this);
	}
}
void D3D12Buffer::UpdateConstantBuffer(void * data, int offset)
{
	if (CrossDevice)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			CBV[i]->UpdateCBV(data, offset, TotalByteSize);
		}
	}
	else
	{
		CBV[0]->UpdateCBV(data, offset, TotalByteSize);
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

GPUResource * D3D12Buffer::GetResource()
{
	return m_DataBuffer;
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
	D3D12CommandList* d3dlist = (D3D12CommandList*)list;
	if (BufferAccesstype != EBufferAccessType::GPUOnly)//gpu buffer states are explictily managed by render code
	{
		m_DataBuffer->SetResourceState(d3dlist->GetCommandList(), PostUploadState);
	}
	SRVBufferHeap->BindHeap(d3dlist->GetCommandList());
	if (list->IsComputeList())
	{
		d3dlist->GetCommandList()->SetComputeRootDescriptorTable(RSSlot, SRVBufferHeap->GetGpuAddress(0));
	}
	else
	{
		d3dlist->GetCommandList()->SetGraphicsRootDescriptorTable(RSSlot, SRVBufferHeap->GetGpuAddress(0));
	}
}

void D3D12Buffer::SetBufferState(RHICommandList * list, EBufferResourceState::Type State)
{
	D3D12CommandList* d3dlist = (D3D12CommandList*)list;
#if _DEBUG
	D3D12_RESOURCE_STATES s = D3D12Helpers::ConvertBufferResourceState(State);
#endif
	m_DataBuffer->SetResourceState(d3dlist->GetCommandList(), D3D12Helpers::ConvertBufferResourceState(State));
}

void D3D12Buffer::SetupBufferSRV()
{
	if (SRVBufferHeap == nullptr)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = ElementCount;
		srvDesc.Buffer.StructureByteStride = ElementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		SRVBufferHeap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		Device->GetDevice()->CreateShaderResourceView(m_DataBuffer->GetResource(), &srvDesc, SRVBufferHeap->GetCPUAddress(0));
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
	TotalByteSize = ByteSize;//mazsize
	ID3D12Resource* TempRes = nullptr;
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(TotalByteSize, Desc.AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&TempRes)));
	m_DataBuffer = new GPUResource(TempRes, D3D12_RESOURCE_STATE_COPY_DEST);
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
	//This Buffer Will Have Data Changed Every frame so no need to transiton to only gpu.
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
	m_DataBuffer = new GPUResource(TempRes, D3D12_RESOURCE_STATE_GENERIC_READ);
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
		m_DataBuffer = new GPUResource(TempRes, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
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
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = TotalByteSize;
	D3D12Helpers::NameRHIObject(m_DataBuffer, this);
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

void D3D12RHIUAV::CreateUAVFromRHIBuffer(RHIBuffer * target)
{
	D3D12Buffer* d3dtarget = (D3D12Buffer*)target;
	ensure(target->CurrentBufferType == ERHIBufferType::GPU);
	ensure(d3dtarget->CheckDevice(Device->GetDeviceIndex()));
	Heap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
#if NAME_RHI_PRIMS
	SetDebugName(d3dtarget->GetDebugName());
#endif
	D3D12Helpers::NameRHIObject(Heap, this);
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	destTextureUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	destTextureUAVDesc.Buffer.FirstElement = 0;
	destTextureUAVDesc.Buffer.NumElements = d3dtarget->ElementCount;
	destTextureUAVDesc.Buffer.StructureByteStride = d3dtarget->ElementSize;
	destTextureUAVDesc.Buffer.CounterOffsetInBytes = d3dtarget->CounterOffset;
	destTextureUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	Device->GetDevice()->CreateUnorderedAccessView(d3dtarget->GetResource()->GetResource(), d3dtarget->GetResource()->GetResource(), &destTextureUAVDesc, Heap->GetCPUAddress(0));
}

void D3D12RHIUAV::CreateUAVFromTexture(BaseTexture * target)
{
	D3D12Texture* D3DTarget = (D3D12Texture*)target;
	ensure(D3DTarget->CheckDevice(Device->GetDeviceIndex()));
	Heap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
#if 	NAME_RHI_PRIMS
	SetDebugName(D3DTarget->GetDebugName());
#endif
	D3D12Helpers::NameRHIObject(Heap, this);
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
#if NAME_RHI_PRIMS
	SetDebugName(D3DTarget->GetDebugName());
#endif
	D3D12Helpers::NameRHIObject(Heap, this);
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
	if (list->IsComputeList())
	{
		DXList->GetCommandList()->SetComputeRootDescriptorTable(slot, Heap->GetGpuAddress(0));
	}
	else
	{
		DXList->GetCommandList()->SetGraphicsRootDescriptorTable(slot, Heap->GetGpuAddress(0));
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Texture Array
D3D12RHITextureArray::D3D12RHITextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
{
	AddCheckerRef(D3D12RHITextureArray, this);
	Heap = new DescriptorHeap(device, NumEntries, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12Helpers::NameRHIObject(Heap, this);
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
