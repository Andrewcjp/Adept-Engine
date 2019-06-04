#include "D3D12CommandList.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12CBV.h"
#include "D3D12DeviceContext.h"
#include "D3D12Framebuffer.h"
#include "DescriptorHeap.h"
#include "GPUResource.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/Core/RenderBaseTypes.h"
#include "DescriptorHeapManager.h"
#include "Descriptor.h"
#include "DescriptorGroup.h"
#include "D3D12RHI.h"
#include "Raytracing/D3D12StateObject.h"
#if FORCE_RENDER_PASS_USE
#define CHECKRPASS() ensure(IsInRenderPass);
#else
#define CHECKRPASS() 
#endif
D3D12CommandList::D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType) :RHICommandList(ListType, inDevice)
{
	AddCheckerRef(D3D12CommandList, this);
	mDeviceContext = D3D12RHI::DXConv(inDevice);
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandAllocator(D3D12Helpers::ConvertListType(ListType), IID_PPV_ARGS(&m_commandAllocator[i])));
	}
	if (ListType == ECommandListType::Copy)
	{
		//copy queues don't have pipeline states!
		CreateCommandList();
	}
}

void D3D12CommandList::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12CommandList, this);
	SafeRelease(CurrentCommandList);
	MemoryUtils::DeleteReleaseableCArray(m_commandAllocator, RHI::CPUFrameCount);
	SafeRelease(CommandSig);
}

bool D3D12CommandList::IsOpen()
{
	return m_IsOpen;
}

void D3D12CommandList::ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{
	ensure(CommandSig != nullptr);
	ensure(ArgumentBuffer);
	ID3D12Resource* counterB = nullptr;
	if (CountBuffer != nullptr)
	{
		counterB = D3D12RHI::DXConv(CountBuffer)->GetResource()->GetResource();
	}
	//PushPrimitiveTopology();
	CurrentCommandList->ExecuteIndirect(CommandSig, MaxCommandCount, D3D12RHI::DXConv(ArgumentBuffer)->GetResource()->GetResource(), ArgOffset, counterB, CountBufferOffset);
}

void D3D12CommandList::SetPipelineStateDesc(RHIPipeLineStateDesc& Desc)
{
	if (CurrentPSO != nullptr && CurrentPSO->GetDesc() == Desc)
	{
		return;
	}
	if (CurrentRenderTarget != nullptr)
	{
		//todo: this might cause issues need to check this behavior
		Desc.FrameBufferTarget = CurrentRenderTarget;
	}
	SetPipelineStateObject(Device->GetPSOCache()->GetFromCache(Desc));
}

void D3D12CommandList::BeginRenderPass(RHIRenderPassDesc& info)
{
	RHICommandList::BeginRenderPass(info);
	if (info.DepthSourceBuffer != nullptr)
	{
		info.DepthSourceBuffer->BindDepthWithColourPassthrough(this, info.TargetBuffer);
	}
	else
	{
		SetRenderTarget(info.TargetBuffer);
	}
	if (info.LoadOp == ERenderPassLoadOp::Clear)
	{
		ClearFrameBuffer(info.TargetBuffer);
	}
}

void D3D12CommandList::EndRenderPass()
{
	RHICommandList::EndRenderPass();
}

void D3D12CommandList::AddHeap(DescriptorHeap * heap)
{
	ClearHeaps();
	heaps.push_back(heap);
}

void D3D12CommandList::PushHeaps()
{
	if (heaps.size() == 0)
	{
		return;
	}
	std::vector<ID3D12DescriptorHeap*> ppHeaps;
	for (int i = 0; i < heaps.size(); i++)
	{
		ppHeaps.push_back(heaps[i]->GetHeap());
	}
	CurrentCommandList->SetDescriptorHeaps((UINT)ppHeaps.size(), ppHeaps.data());
}

void D3D12CommandList::ClearHeaps()
{
	heaps.clear();
}

ID3D12GraphicsCommandList4 * D3D12CommandList::GetCMDList4()
{
	return CurrentADVCommandList;
}

void D3D12CommandList::PushPrimitiveTopology()
{
	CHECKRPASS();
	if (IsGraphicsList())
	{
		if (CurrentPSO->GetDesc().RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_LINE)
		{
			CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		}
		else if (CurrentPSO->GetDesc().RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		{
			CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
	}
}

D3D12CommandList::~D3D12CommandList()
{
#if AFTERMATH
	GFSDK_Aftermath_ReleaseContextHandle(AMHandle);
#endif
}

void D3D12CommandList::ResetList()
{
	SCOPE_CYCLE_COUNTER_GROUP("ResetList", "RHI");
	ensure(!m_IsOpen);
	ThrowIfFailed(m_commandAllocator[Device->GetCpuFrameIndex()]->Reset());
	m_IsOpen = true;
	if (CurrentCommandList == nullptr)
	{
		CreateCommandList();
	}
	ensure(CurrentCommandList != nullptr);
	ID3D12PipelineState* PSO = nullptr;
	if (CurrentPSO != nullptr)
	{
		PSO = D3D12RHI::DXConv(CurrentPSO)->PSO;
	}
	ThrowIfFailed(CurrentCommandList->Reset(m_commandAllocator[Device->GetCpuFrameIndex()], PSO));
	HandleStallTimer();
	PushState();
	if (ListType != ECommandListType::Copy)
	{
		mDeviceContext->GetHeapManager()->BindHeap(this);
	}
}

void D3D12CommandList::SetRenderTarget(FrameBuffer * target, int SubResourceIndex)
{
	//if (!IsInRenderPass)
	//{
	//	Log::LogMessage("SetRenderTarget is depreciated use the new render pass API", Log::Warning);
	//}
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
		CurrentRenderTarget = D3D12RHI::DXConv(target);
		ensure(CurrentRenderTarget->CheckDevice(Device->GetDeviceIndex()));
		CurrentRenderTarget->BindBufferAsRenderTarget(CurrentCommandList, SubResourceIndex);
	}
}

void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	PushHeaps();
	ClearHeaps();
	CHECKRPASS();
	ensure(m_IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	PushPrimitiveTopology();
	CurrentCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{
	PushHeaps();
	ClearHeaps();
	CHECKRPASS();
	ensure(m_IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	PushPrimitiveTopology();
	CurrentCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
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
	D3D12Buffer* dbuffer = D3D12RHI::DXConv(buffer);
	dbuffer = IRHISharedDeviceObject<RHIBuffer>::GetObject<D3D12Buffer>(buffer, Device);
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	dbuffer->EnsureResouceInFinalState(GetCommandList());
	CurrentCommandList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
	PushPrimitiveTopology();
}

void D3D12CommandList::SetIndexBuffer(RHIBuffer * buffer)
{
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = D3D12RHI::DXConv(buffer);
	dbuffer = IRHISharedDeviceObject<RHIBuffer>::GetObject<D3D12Buffer>(buffer, Device);
	dbuffer->EnsureResouceInFinalState(GetCommandList());
	CurrentCommandList->IASetIndexBuffer(&dbuffer->m_IndexBufferView);
}


D3D12PipeLineStateObject::D3D12PipeLineStateObject(const RHIPipeLineStateDesc& desc, DeviceContext* con) :RHIPipeLineStateObject(desc)
{
	Device = con;
}

D3D12PipeLineStateObject::~D3D12PipeLineStateObject()
{}

void D3D12PipeLineStateObject::Complie()
{
	if (RHI::GetFrameCount() > 10)
	{
		Log::LogMessage("Created PSO at runtime", Log::Severity::Error);
	}
	ensure(PSO == nullptr);
	ensure(RootSig == nullptr);
	ensure(Desc.ShaderInUse);
	Desc.Build();
	int VertexDesc_ElementCount = 0;
	D3D12Shader* target = D3D12RHI::DXConv(Desc.ShaderInUse->GetShaderProgram());
	ensure(target != nullptr);
	ensure((Desc.ShaderInUse->GetShaderParameters().size() > 0));
	ensure((Desc.ShaderInUse->GetVertexFormat().size() > 0));
	D3D12_INPUT_ELEMENT_DESC* desc;
	D3D12Shader::ParseVertexFormat(Desc.ShaderInUse->GetVertexFormat(), &desc, &VertexDesc_ElementCount);
	D3D12Shader::CreateRootSig(this, Desc.ShaderInUse->GetShaderParameters(), Device, Desc.ShaderInUse->IsComputeShader(), RHISamplerDesc::GetDefault());
	if (Desc.ShaderInUse->IsComputeShader())
	{
		D3D12Shader::CreateComputePipelineShader(this, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Desc, Device);
	}
	else
	{
		D3D12Shader::CreatePipelineShader(this, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Desc, Device);
	}
	PSO->SetName(StringUtils::ConvertStringToWide(Desc.GetString()).c_str());
}

void D3D12PipeLineStateObject::Release()
{
	SafeRelease(PSO);
	SafeRelease(RootSig);
}

void D3D12CommandList::SetPipelineStateObject(RHIPipeLineStateObject* Object)
{
	ensure(Object);
	ensure(Object->GetDevice() == Device);
	Device->UpdatePSOTracker(Object);
	CurrentPSO = Object;
	if (CurrentCommandList == nullptr)
	{
		CreateCommandList();
	}
	PushState();
}

void D3D12CommandList::PushState()
{
	if (IsOpen() && !IsCopyList())
	{
		Device->UpdatePSOTracker(CurrentPSO);
		D3D12PipeLineStateObject* DPSO = D3D12RHI::DXConv(CurrentPSO);
		if (DPSO != nullptr)
		{
			CurrentCommandList->SetPipelineState(DPSO->PSO);
			if (IsGraphicsList())
			{
				CurrentCommandList->SetGraphicsRootSignature(DPSO->RootSig);
			}
			else
			{
				CurrentCommandList->SetComputeRootSignature(DPSO->RootSig);
			}
		}
	}
}

void D3D12CommandList::CreateCommandList()
{
	if (CurrentCommandList != nullptr)
	{
		return;
	}
	ID3D12PipelineState* PSO = nullptr;
	if (CurrentPSO != nullptr)
	{
		PSO = D3D12RHI::DXConv(CurrentPSO)->PSO;
	}
	if (ListType == ECommandListType::Graphics)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[Device->GetCpuFrameIndex()], PSO, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Compute)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocator[Device->GetCpuFrameIndex()], PSO, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Copy)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_COPY, m_commandAllocator[Device->GetCpuFrameIndex()], nullptr, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	if (mDeviceContext->SupportsCommandList4())
	{
		CurrentCommandList->QueryInterface(IID_PPV_ARGS(&CurrentADVCommandList));
	}
	PushState();
	D3D12Helpers::NameRHIObject(CurrentCommandList, this);
#if AFTERMATH
	GFSDK_Aftermath_DX12_CreateContextHandle(CurrentCommandList, &AMHandle);
	D3D12RHI::Get()->handles.push_back(AMHandle);
	const char* s = "asdasd";
	GFSDK_Aftermath_SetEventMarker(AMHandle, nullptr, 0);
#endif
}

void D3D12CommandList::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{
	ensure(ListType == ECommandListType::Compute);
	CurrentCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void D3D12CommandList::CopyResourceToSharedMemory(FrameBuffer * Buffer)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* buffer = D3D12RHI::DXConv(Buffer);
	ensure(Device == buffer->GetDevice());
	buffer->CopyToHostMemory(CurrentCommandList);
}

void D3D12CommandList::CopyResourceFromSharedMemory(FrameBuffer * Buffer)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* buffer = D3D12RHI::DXConv(Buffer);
	ensure(Device == buffer->GetTargetDevice());
	buffer->CopyFromHostMemory(CurrentCommandList);
}

void D3D12CommandList::ClearFrameBuffer(FrameBuffer * buffer)
{
	ensure(buffer);
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12RHI::DXConv(buffer)->ClearBuffer(CurrentCommandList);
}

void D3D12CommandList::UAVBarrier(RHIUAV * target)
{
	//The resource can be NULL, which indicates that any UAV access could require the barrier.
	if (target != nullptr)
	{
		ensure(!target->IsPendingKill());
		D3D12RHIUAV* dtarget = D3D12RHI::DXConv(target);
		CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->m_UAV));
		CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->UAVCounter));
	}
	else
	{
		CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
	}
}

void D3D12CommandList::SetUpCommandSigniture(int commandSize, bool Dispatch)
{
	//#RHI: RHI indirect commands
	//#Note: Care full there was an address out of scope issue here/
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	ZeroMemory(&commandSignatureDesc, sizeof(commandSignatureDesc));
	if (Dispatch)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		ensure(commandSize % 4 == 0);

		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = commandSize;
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandSignature(&commandSignatureDesc, Dispatch ? nullptr : ((D3D12PipeLineStateObject*)CurrentPSO)->RootSig, IID_PPV_ARGS(&CommandSig)));
		NAME_D3D12_OBJECT(CommandSig);
	}
	else
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
		ZeroMemory(&argumentDescs, sizeof(argumentDescs));
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argumentDescs[0].Constant.RootParameterIndex = 0;
		argumentDescs[0].Constant.Num32BitValuesToSet = 1;
		argumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
		argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

		ensure(commandSize % 4 == 0);
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = commandSize;
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandSignature(&commandSignatureDesc, Dispatch ? nullptr : ((D3D12PipeLineStateObject*)CurrentPSO)->RootSig, IID_PPV_ARGS(&CommandSig)));
		NAME_D3D12_OBJECT(CommandSig);
	}
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

CMDListType* D3D12CommandList::GetCommandList()
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
	ensure(m_IsOpen);
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
	D3D12FrameBuffer* DBuffer = D3D12RHI::DXConv(buffer);
	if (ListType == ECommandListType::Compute)
	{
		ensure(DBuffer->IsReadyForCompute());
	}
	if (Device->GetStateCache()->RenderTargetCheckAndUpdate(buffer))
	{
		return;
	}
	ensure(DBuffer->CheckDevice(Device->GetDeviceIndex()));
	DBuffer->BindBufferToTexture(CurrentCommandList, slot, Resourceindex, Device, (ListType == ECommandListType::Compute));
}



void D3D12CommandList::TraceRays(const RHIRayDispatchDesc& desc)
{
	ensure(CurrentRTState);
	//#DXR: todo
	CurrentRTState->Trace(desc, this, CurrentRenderTarget);
}

void D3D12CommandList::SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct)
{
	ensure(CurrentRTState);
	CurrentRTState->High = Struct;
}

void D3D12CommandList::SetStateObject(RHIStateObject* Object)
{
	CurrentRTState = D3D12RHI::DXConv(Object);
}

void D3D12CommandList::SetTexture(BaseTextureRef texture, int slot)
{
	ensure(texture != nullptr);
	ensure(!texture->IsPendingKill());
	Texture = D3D12RHI::DXConv(texture.Get());
	Texture = IRHISharedDeviceObject<BaseTexture>::GetObject<D3D12Texture>(texture.Get(), Device);
	ensureMsgf(Texture->CheckDevice(Device->GetDeviceIndex()), "Attempted to Bind texture that is not on this device");
	if (Device->GetStateCache()->TextureCheckAndUpdate(texture.Get(), slot))
	{
		return;
	}
	if (CurrentCommandList != nullptr)
	{
		Texture->BindToSlot(this, slot);
	}
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot)
{
	ensure(!buffer->IsPendingKill());
	D3D12Buffer* d3Buffer = D3D12RHI::DXConv(buffer);
	ensure(d3Buffer->CheckDevice(Device->GetDeviceIndex()));
	d3Buffer->SetConstantBufferView(offset, CurrentCommandList, Slot, ListType == ECommandListType::Compute, Device->GetDeviceIndex());
}

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
	D3D12CommandList* d3dlist = D3D12RHI::DXConv(list);
	if (BufferAccesstype != EBufferAccessType::GPUOnly)//gpu buffer states are explicitly managed by render code
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
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.NumElements = ElementCount;
		srvDesc.Buffer.StructureByteStride = ElementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
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
	Device = D3D12RHI::DXConv(inDevice);
	AddCheckerRef(D3D12RHIUAV, this);
}
void D3D12RHIUAV::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12RHIUAV, this);
	SafeRelease(UAVDescriptor);
	SafeRelease(UAVCounter);
	SafeRelease(m_UAV);
}

D3D12RHIUAV::~D3D12RHIUAV()
{}

void D3D12RHIUAV::CreateUAVFromRHIBuffer(RHIBuffer * target)
{
	D3D12Buffer* d3dtarget = D3D12RHI::DXConv(target);
	ensure(target->CurrentBufferType == ERHIBufferType::GPU);
	ensure(d3dtarget->CheckDevice(Device->GetDeviceIndex()));
	if (UAVDescriptor == nullptr)
	{
		UAVDescriptor = Device->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
#if NAME_RHI_PRIMS
	SetDebugName(d3dtarget->GetDebugName());
#endif

	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	destTextureUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	destTextureUAVDesc.Buffer.FirstElement = 0;
	destTextureUAVDesc.Buffer.NumElements = d3dtarget->ElementCount;
	destTextureUAVDesc.Buffer.StructureByteStride = d3dtarget->ElementSize;
	destTextureUAVDesc.Buffer.CounterOffsetInBytes = d3dtarget->CounterOffset;
	destTextureUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	UAVDescriptor->CreateUnorderedAccessView(d3dtarget->GetResource()->GetResource(), d3dtarget->GetResource()->GetResource(), &destTextureUAVDesc);
}

void D3D12RHIUAV::CreateUAVFromTexture(BaseTexture * target)
{
	D3D12Texture* D3DTarget = D3D12RHI::DXConv(target);
	ensure(D3DTarget->CheckDevice(Device->GetDeviceIndex()));
	if (UAVDescriptor == nullptr)
	{
		UAVDescriptor = Device->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
#if NAME_RHI_PRIMS
	SetDebugName(D3DTarget->GetDebugName());
#endif

	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	destTextureUAVDesc.Format = ((D3D12Texture*)target)->GetResource()->GetDesc().Format;
	destTextureUAVDesc.Texture2D.MipSlice = 1;
	//todo: Counter UAV?
	UAVDescriptor->CreateUnorderedAccessView(((D3D12Texture*)target)->GetResource(), UAVCounter, &destTextureUAVDesc);
}

void D3D12RHIUAV::CreateUAVFromFrameBuffer(FrameBuffer * target, int mip)
{
	D3D12FrameBuffer* D3DTarget = D3D12RHI::DXConv(target);
	ensure(D3DTarget->CheckDevice(Device->GetDeviceIndex()));
	if (UAVDescriptor == nullptr)
	{
		UAVDescriptor = Device->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
#if NAME_RHI_PRIMS
	SetDebugName(D3DTarget->GetDebugName());
#endif
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(target->GetDescription().RTFormats[0]);
	destTextureUAVDesc.Texture2D.MipSlice = mip;
	UAVDescriptor->CreateUnorderedAccessView(((D3D12FrameBuffer*)target)->GetResource(0)->GetResource(), UAVCounter, &destTextureUAVDesc);
}

void D3D12RHIUAV::Bind(RHICommandList * list, int slot)
{
	ensure(Device == list->GetDevice());
	D3D12CommandList* DXList = D3D12RHI::DXConv(list);
	if (list->IsComputeList())
	{
		DXList->GetCommandList()->SetComputeRootDescriptorTable(slot, UAVDescriptor->GetGPUAddress(0));
	}
	else
	{
		DXList->GetCommandList()->SetGraphicsRootDescriptorTable(slot, UAVDescriptor->GetGPUAddress(0));
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Texture Array
D3D12RHITextureArray::D3D12RHITextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
{
	AddCheckerRef(D3D12RHITextureArray, this);
	Device = D3D12RHI::DXConv(device);
	Desc = Device->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumEntries);
}

D3D12RHITextureArray::~D3D12RHITextureArray()
{}

//Add a frame buffer to this heap and ask it to create one in our heap
void D3D12RHITextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(Buffer);
	ensure(dBuffer->CheckDevice(Device->GetDeviceIndex()));
	LinkedBuffers.push_back(dBuffer);
	dBuffer->CreateSRVInHeap(slot, Desc);
	NullHeapDesc = dBuffer->GetSrvDesc(0);
}

void D3D12RHITextureArray::BindToShader(RHICommandList * list, int slot)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(list);
	ensure(DXList->GetDevice() == Device);
	for (int i = 0; i < LinkedBuffers.size(); i++)
	{
		LinkedBuffers[i]->ReadyResourcesForRead(DXList->GetCommandList());
	}
	DXList->GetCommandList()->SetGraphicsRootDescriptorTable(slot, Desc->GetGPUAddress(0));
}

//Makes a descriptor Null Using the first frame buffers Description
void D3D12RHITextureArray::SetIndexNull(int TargetIndex, FrameBuffer* Buffer /*= nullptr*/)
{
	if (Buffer != nullptr)
	{
		D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(Buffer);
		NullHeapDesc = dBuffer->GetSrvDesc(0);
	}
	if (NullHeapDesc.Format == DXGI_FORMAT_UNKNOWN)
	{
		Log::LogMessage("Texture Array Slot Cannot be set null without format", Log::Error);
		return;
	}
	Desc->CreateShaderResourceView(nullptr, &NullHeapDesc, TargetIndex);
	//Device->GetDevice()->CreateShaderResourceView(nullptr, &NullHeapDesc, Desc->GetCPUAddress(TargetIndex));
}

void D3D12RHITextureArray::SetFrameBufferFormat(RHIFrameBufferDesc & desc)
{
	NullHeapDesc = D3D12FrameBuffer::GetSrvDesc(0, desc);
}

void D3D12RHITextureArray::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12RHITextureArray, this);
	SafeRelease(Desc);
}

void D3D12RHITextureArray::Clear()
{
	if (LinkedBuffers.size() == 0)
	{
		return;
	}
	for (int i = 0; i < LinkedBuffers.size(); i++)
	{
		SetIndexNull(i);//clear all buffers;
	}
	LinkedBuffers.clear();
}


