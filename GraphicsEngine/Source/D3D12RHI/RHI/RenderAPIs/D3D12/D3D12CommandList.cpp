#include "D3D12CommandList.h"
#include "Core/Performance/PerfManager.h"
#include "D3D12DeviceContext.h"
#include "D3D12Framebuffer.h"
#include "DescriptorHeap.h"
#include "GPUResource.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/Core/RenderBaseTypes.h"
#include "DescriptorHeapManager.h"
#include "DXDescriptor.h"
#include "D3D12RHI.h"
#include "Raytracing/D3D12StateObject.h"
#include "D3D12Buffer.h"
#include "DescriptorCache.h"
#include "RHI/RHITexture.h"
#include "D3D12RHITexture.h"
#if PIX_ENABLED
#include "pix3.h"
#endif
#if FORCE_RENDER_PASS_USE
#define CHECKRPASS() ensure(IsInRenderPass);
#else
#define CHECKRPASS() 
#endif

D3D12CommandList::D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType) :RHICommandList(ListType, inDevice)
{
	mDeviceContext = D3D12RHI::DXConv(inDevice);
	//CommandAlloc = new CommandAllocator(ListType, mDeviceContext);
	if (ListType == ECommandListType::Copy)
	{
		//copy queues don't have pipeline states!
		CreateCommandList();
	}
	PerfManager::Get()->AddTimer("Resource Barrier Calls", "RHI");
}

void D3D12CommandList::Release()
{
	IRHIResourse::Release();
	SafeRelease(CurrentCommandList);
	//SafeDelete(CommandAlloc);
	//	SafeRelease(CommandSig);
}

bool D3D12CommandList::IsOpen()const
{
	return m_IsOpen;
}

void D3D12CommandList::SetPipelineStateDesc(const RHIPipeLineStateDesc& Desc)
{
	//	ensure(IsOpen());
	if (ListType != ECommandListType::RayTracing)
	{
		if (CurrentPSO != nullptr && CurrentPSO->GetDesc() == Desc)
		{
			return;
		}
	}
	RHIPipeLineStateDesc TDesc = Desc;
	if (CurrentRenderTarget != nullptr)
	{
		//todo: this might cause issues need to check this behavior
		TDesc.RenderTargetDesc = CurrentRenderTarget->GetPiplineRenderDesc();
	}
	SetPipelineStateObject(Device->GetPSOCache()->GetFromCache(TDesc));
}

void D3D12CommandList::BeginRenderPass(const RHIRenderPassDesc& info)
{
	FlushBarriers();
	CommandCount++;
	ensure(IsOpen());
	RHICommandList::BeginRenderPass(info);
	if (info.TargetSwapChain)
	{
		SetScreenBackBufferAsRT();
		if (info.LoadOp == ERenderPassLoadOp::Clear)
		{
			ClearScreen();
		}
	}
	else
	{
		if (info.DepthSourceBuffer != nullptr)
		{
			info.DepthSourceBuffer->BindDepthWithColourPassthrough(this, info.TargetBuffer);
		}
		else
		{
			SetRenderTarget(info.TargetBuffer, info.SubResourceIndex);
		}
		if (info.LoadOp == ERenderPassLoadOp::Clear)
		{
			ClearFrameBuffer(info.TargetBuffer);
		}
	}
}

void D3D12CommandList::EndRenderPass()
{
	RHICommandList::EndRenderPass();
	SetRenderTarget(nullptr);
	CommandCount++;
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
	ID3D12DescriptorHeap* heap[] = { heaps[0]->GetHeap() };
	CurrentCommandList->SetDescriptorHeaps((UINT)1, heap);
	CurrentBoundHeap = heap[0];
}

void D3D12CommandList::ClearHeaps()
{
	heaps.clear();
}
#if WIN10_1809
ID3D12GraphicsCommandList4 * D3D12CommandList::GetCMDList4()
{
	return CurrentADVCommandList;
}
#endif
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
		else if (CurrentPSO->GetDesc().RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_POINT)
		{
			CurrentCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		}
	}
}
#if WIN10_1903
void D3D12CommandList::SetVRSShadingRateNative(VRX_SHADING_RATE::type Rate)
{
	ensure(mDeviceContext->GetCaps().VRSSupport != EVRSSupportType::None);
	ensure(CmdList5 != nullptr);
	CmdList5->RSSetShadingRate((D3D12_SHADING_RATE)Rate, nullptr);
}

void D3D12CommandList::SetVRSShadingRateImageNative(RHITexture* Target)
{
	D3D12_SHADING_RATE_COMBINER combiner[D3D12_RS_SET_SHADING_RATE_COMBINER_COUNT] = {};
	combiner[0] = D3D12_SHADING_RATE_COMBINER::D3D12_SHADING_RATE_COMBINER_PASSTHROUGH;
	combiner[1] = D3D12_SHADING_RATE_COMBINER::D3D12_SHADING_RATE_COMBINER_OVERRIDE;
	CmdList5->RSSetShadingRate(D3D12_SHADING_RATE::D3D12_SHADING_RATE_1X1, combiner);
	ensure(mDeviceContext->GetCaps().VRSSupport != EVRSSupportType::None);
	ensure(CmdList5 != nullptr);
	GPUResource* Resource = D3D12RHI::DXConv(Target)->GetResource();
	Resource->SetResourceState(this, D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE);
	ensure(Resource->GetCurrentState() == D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE);
	CmdList5->RSSetShadingRateImage(Resource->GetResource());
}
#endif
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
	CommandAlloc = mDeviceContext->GetAllocator(this);
	CommandAlloc->Reset();
	CommandAlloc->SetUser(this);
	m_IsOpen = true;
	if (CurrentCommandList == nullptr)
	{
		CreateCommandList();
	}
	ID3D12PipelineState* PSO = nullptr;
	if (CurrentPSO != nullptr)
	{
		PSO = D3D12RHI::DXConv(CurrentPSO)->PSO;
	}
	if (ListType == ECommandListType::Copy)
	{
		ThrowIfFailed(GetCopyList()->Reset(GetCommandAllocator(), PSO));
	}
	else
	{
		ThrowIfFailed(CurrentCommandList->Reset(GetCommandAllocator(), PSO));
	}
	HandleStallTimer();
	PushState();
	if (ListType != ECommandListType::Copy)
	{
		mDeviceContext->GetHeapManager()->BindHeap(this);
	}
	RootSigniture.Reset();
	DrawDispatchCount = 0;
	GPUWorkEstimate = 0;
	CommandCount = 0;
}

ID3D12CommandAllocator* D3D12CommandList::GetCommandAllocator()
{
	return CommandAlloc->GetAllocator();
}

void D3D12CommandList::SetRenderTarget(FrameBuffer * target, int SubResourceIndex)
{
	ensure(ListType == ECommandListType::Graphics || IsRaytracingList());
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
		CurrentRenderTarget->BindBufferAsRenderTarget(this, SubResourceIndex);
	}
}

void D3D12CommandList::PrepareforDraw()
{
	SCOPE_CYCLE_COUNTER_GROUP("PrepareforDraw", "RHI");
	FlushBarriers();
	mDeviceContext->GetHeapManager()->CheckAndRealloc(RootSigniture.GetMaxDescriptorsNeeded(), this);
	PushPrimitiveTopology();
	PushHeaps();
	for (int i = 0; i < RootSigniture.GetNumBinds(); i++)
	{
		const RSBind* bind = RootSigniture.GetBind(i);
		DebugEnsure(bind->BindParm);
		DXDescriptor* desc = nullptr;
		//DebugEnsure(bind->IsBound());
		if (bind->BindType == ERSBindType::Texture && bind->IsBound())
		{
			desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
			if (IsGraphicsList())
			{
				GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
			else
			{
				GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
		}
		else if (bind->BindType == ERSBindType::FrameBuffer && bind->IsBound())
		{
			desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
			if (IsGraphicsList())
			{
				GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
			else
			{
				GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
		}
		else if (bind->BindType == ERSBindType::BufferSRV)
		{
			ensure(bind->View.ViewType != EViewType::Limit);
			if (bind->IsBound())
			{
				desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
				if (bind->BufferTarget != nullptr)
				{
					D3D12RHI::DXConv(bind->BufferTarget)->PushDataUpLoad(this);
				}
			}
			if (IsGraphicsList())
			{
				GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
			else
			{
				GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}

		}
		else if (bind->BindType == ERSBindType::TextureArray)
		{
			if (bind->IsBound())
			{
				desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
				if (IsGraphicsList())
				{
					GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
				}
				else
				{
					GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
				}
			}
		}
		else if (bind->BindType == ERSBindType::UAV)
		{
			if (bind->IsBound())
			{
				ensure(bind->View.ViewType != EViewType::Limit);
				desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
				if (IsGraphicsList())
				{
					GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
				}
				else
				{
					GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
				}
			}
		}
		else if (bind->BindType == ERSBindType::Texture2)
		{
			if (bind->IsBound())
			{
				ensure(bind->View.ViewType != EViewType::Limit);
				desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
				if (IsGraphicsList())
				{
					GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
				}
				else
				{
					GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
				}
			}
		}
	}
	//if a resize occurred we need to set the heap to the cmd list again
	mDeviceContext->GetHeapManager()->RebindHeap(this);
	DrawDispatchCount++;
	CommandCount++;
}

void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	PrepareforDraw();
	CHECKRPASS();
	ensure(m_IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	CurrentCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::DrawIndexedPrimitive(uint IndexCountPerInstance, uint InstanceCount, uint StartIndexLocation, uint BaseVertexLocation, uint StartInstanceLocation)
{
	PrepareforDraw();
	CHECKRPASS();
	ensure(m_IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	CurrentCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{
	ensure(ListType == ECommandListType::Graphics);
	CD3DX12_VIEWPORT m_viewport = CD3DX12_VIEWPORT((FLOAT)MinX, (FLOAT)MinY, (FLOAT)MaxX, (FLOAT)MaxY);
	CurrentCommandList->RSSetViewports(1, &m_viewport);
	CD3DX12_RECT ScissorR = CD3DX12_RECT(MinX, MinY, MaxX, MaxY);
	CurrentCommandList->RSSetScissorRects(1, &ScissorR);
}

void D3D12CommandList::Execute(DeviceContextQueue::Type Target)
{
	SCOPE_CYCLE_COUNTER_GROUP("Execute", "RHI");
	FlushBarriers();
	if (Target == DeviceContextQueue::LIMIT)
	{
		switch (ListType)
		{
		case ECommandListType::Graphics:
			Target = DeviceContextQueue::Graphics;
			break;
		case ECommandListType::RayTracing:
		case ECommandListType::Compute:
			Target = DeviceContextQueue::Compute;
			break;
		case ECommandListType::Copy:
			Target = DeviceContextQueue::Copy;
			break;
		}
	}
	if (IsOpen())
	{
		if (ListType == ECommandListType::Copy)
		{
			ThrowIfFailed(GetCopyList()->Close());
		}
		else
		{
			ThrowIfFailed(CurrentCommandList->Close());
		}
	}
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
		mDeviceContext->ExecuteCopyCommandList(GetCopyList());
	}
	else if (Target == DeviceContextQueue::InterCopy)
	{
		mDeviceContext->ExecuteInterGPUCopyCommandList(CurrentCommandList);
	}
	m_IsOpen = false;
	CommandAlloc->SetUser(nullptr);
	if (RHI::GetFrameCount() % 100 == 0 && false)
	{
		if (ListType != ECommandListType::Copy)
		{
			LogEnsureMsgf(DrawDispatchCount, "No dispatch/draw commands in list");
		}
		LogEnsureMsgf(CommandCount, "Executing empty command list");
	}
}

void D3D12CommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	FlushBarriers();
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = D3D12RHI::DXConv(buffer);
	dbuffer = IRHISharedDeviceObject<RHIBuffer>::GetObject<D3D12Buffer>(buffer, Device);
	ensure(dbuffer->CheckDevice(Device->GetDeviceIndex()));
	dbuffer->EnsureResouceInFinalState(this);
	CurrentCommandList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
	PushPrimitiveTopology();
	CommandCount++;
}

void D3D12CommandList::SetIndexBuffer(RHIBuffer * buffer)
{
	FlushBarriers();
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12Buffer* dbuffer = D3D12RHI::DXConv(buffer);
	dbuffer = IRHISharedDeviceObject<RHIBuffer>::GetObject<D3D12Buffer>(buffer, Device);
	dbuffer->EnsureResouceInFinalState(this);
	CurrentCommandList->IASetIndexBuffer(&dbuffer->m_IndexBufferView);
	CommandCount++;
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

	D3D12_INPUT_ELEMENT_DESC* desc;
	if (Desc.InputLayout.Elements.size() != 0)
	{
		D3D12Shader::ParseVertexFormat(Desc.InputLayout.Elements, &desc, &VertexDesc_ElementCount);
	}
	else
	{
		D3D12Shader::ParseVertexFormat(Desc.ShaderInUse->GetVertexFormat(), &desc, &VertexDesc_ElementCount);
	}
	D3D12Shader::CreateRootSig(&this->RootSig, Desc.ShaderInUse->GetShaderParameters(), Device, Desc.ShaderInUse->IsComputeShader(), RHISamplerDesc::GetDefault());
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
	RootSigniture.SetRootSig(Object->GetDesc().ShaderInUse->GetShaderParameters());
	RootSigniture.Reset();
	CommandCount++;
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
	if (CommandAlloc == nullptr)
	{
		CommandAlloc = mDeviceContext->GetAllocator(this);
	}
	ID3D12PipelineState* PSO = nullptr;
	if (CurrentPSO != nullptr)
	{
		PSO = D3D12RHI::DXConv(CurrentPSO)->PSO;
	}
	if (ListType == ECommandListType::Graphics)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_DIRECT, GetCommandAllocator(), PSO, ID_PASS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Compute || IsRaytracingList())
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_COMPUTE, GetCommandAllocator(), PSO, ID_PASS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Copy)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12RHIConfig::CopyListQueueType, GetCommandAllocator(), nullptr, ID_PASS(&CopyListPtr)));
		ThrowIfFailed(CopyListPtr->Close());
#if SUPPORT_DXGI
		CurrentCommandList = CopyListPtr;
#endif
	}
	if (CurrentCommandList != nullptr)
	{
#if WIN10_1809
		if (mDeviceContext->SupportsCommandList4())
		{
			CurrentCommandList->QueryInterface(ID_PASS(&CurrentADVCommandList));
		}
#endif
#if WIN10_1903
		CurrentCommandList->QueryInterface(ID_PASS(&CmdList5));
#endif

		CurrentCommandList->QueryInterface(ID_PASS(&CommandList1));
#if defined(PLATFORM_WINDOWS) && !BUILD_SHIPPING
		CurrentCommandList->QueryInterface(ID_PASS(&DebugList));
#endif
		D3D12Helpers::NameRHIObject(CurrentCommandList, this);
	}
	PushState();
#if AFTERMATH
	GFSDK_Aftermath_DX12_CreateContextHandle(CurrentCommandList, &AMHandle);
	D3D12RHI::Get()->handles.push_back(AMHandle);
	const char* s = "asdasd";
	GFSDK_Aftermath_SetEventMarker(AMHandle, nullptr, 0);
#endif
}

void D3D12CommandList::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{
	PrepareforDraw();
	ensure(ListType == ECommandListType::Compute || ListType == ECommandListType::RayTracing);
	CurrentCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void D3D12CommandList::CopyResourceToSharedMemory(FrameBuffer * Buffer)
{
	ensure(!Buffer->IsPendingKill());
	//	D3D12FrameBuffer* buffer = D3D12RHI::DXConv(Buffer);
		//ensure(Device == buffer->GetDevice());
		//buffer->CopyToHostMemory(CurrentCommandList);
}

void D3D12CommandList::CopyResourceFromSharedMemory(FrameBuffer * Buffer)
{
	ensure(!Buffer->IsPendingKill());
	//D3D12FrameBuffer* buffer = D3D12RHI::DXConv(Buffer);
	//ensure(Device == buffer->GetTargetDevice());
	//buffer->CopyFromHostMemory(CurrentCommandList);
}

void D3D12CommandList::ClearFrameBuffer(FrameBuffer * buffer)
{
	ensure(buffer);
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics);
	D3D12RHI::DXConv(buffer)->ClearBuffer(this);
}

void D3D12CommandList::UAVBarrier(FrameBuffer* target)
{
	D3D12FrameBuffer* dtarget = D3D12RHI::DXConv(target);
	AddTransition(CD3DX12_RESOURCE_BARRIER::UAV(dtarget->GetResource(0)->GetResource()));
	CommandCount++;
}

void D3D12CommandList::UAVBarrier(RHIBuffer* target)
{
	D3D12Buffer* dtarget = D3D12RHI::DXConv(target);
	AddTransition(CD3DX12_RESOURCE_BARRIER::UAV(dtarget->GetResource()->GetResource()));
	CommandCount++;
}

void D3D12CommandList::UAVBarrier(RHITexture* target)
{
	D3D12RHITexture* dtarget = D3D12RHI::DXConv(target);
	AddTransition(CD3DX12_RESOURCE_BARRIER::UAV(dtarget->GetResource()->GetResource()));
	CommandCount++;
}

CopyCMDListType * D3D12CommandList::GetCopyList()
{
	return CopyListPtr;
}

RHIRootSigniture * D3D12CommandList::GetRootSig()
{
	return &RootSigniture;
}

void D3D12CommandList::SetTexture2(RHITexture* t, int slot, const RHIViewDesc& view)
{
	FlushBarriers();
	RootSigniture.SetTexture2(slot, t, view);
	CommandCount++;
}

void D3D12CommandList::SetStencilRef(uint value)
{
	ensure(IsGraphicsList());
	if (CurrentPSO != nullptr)
	{
		LogEnsure(CurrentPSO->GetDesc().DepthStencilState.StencilEnable);
	}
	CurrentCommandList->OMSetStencilRef(value);
	CommandCount++;
}

void D3D12CommandList::FlushBarriers()
{
	if (QueuedBarriers.size() == 0)
	{
		return;
	}
	GetCommandList()->ResourceBarrier(QueuedBarriers.size(), QueuedBarriers.data());
	QueuedBarriers.clear();
	PerfManager::Get()->AddToCountTimer("Resource Barrier Calls", 1);
	CommandCount++;
}

void D3D12CommandList::AddTransition(D3D12_RESOURCE_BARRIER transition)
{
	QueuedBarriers.push_back(transition);
	//FlushBarriers();
}

void D3D12CommandList::ClearUAVFloat(RHIBuffer* buffer)
{
	//	CmdList5->ClearUnorderedAccessViewFloat()
}

void D3D12CommandList::ClearUAVFloat(RHITexture* buffer, glm::vec4 ClearColour)
{
	FlushBarriers();
	D3D12RHITexture* DBuffer = D3D12RHI::DXConv(buffer);
	RSBind bind;
	bind.BindType = ERSBindType::UAV;
	bind.Texture2 = buffer;
	bind.View = RHIViewDesc::DefaultUAV();
	DXDescriptor* desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(&bind);
	DXDescriptor* CPUDesc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->FindInCacheHeap(&bind);
	float Values[4] = { ClearColour.x,ClearColour.y,ClearColour.z,ClearColour.w };
	GetCommandList()->ClearUnorderedAccessViewFloat(desc->GetGPUAddress(), CPUDesc->GetCPUAddress(), DBuffer->GetResource()->GetResource(), Values, 0, nullptr);
}

void D3D12CommandList::ClearUAVUint(RHIBuffer* buffer)
{
	D3D12Buffer* DBuffer = D3D12RHI::DXConv(buffer);
	RSBind bind;
	bind.BindType = ERSBindType::UAV;
	bind.BufferTarget = buffer;
	bind.View = RHIViewDesc::DefaultUAV();
	DXDescriptor* desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(&bind);
	uint Values[4] = { 0,0,0,0 };
	GetCommandList()->ClearUnorderedAccessViewUint(desc->GetGPUAddress(), desc->GetCPUAddress(), DBuffer->GetResource()->GetResource(), Values, 0, nullptr);
}

void D3D12CommandList::ClearUAVUint(RHITexture* buffer)
{
	//#dxtodo: fix this to target all (ish)
	D3D12RHITexture* DBuffer = D3D12RHI::DXConv(buffer);
	RSBind bind;
	bind.BindType = ERSBindType::UAV;
	bind.Texture2 = buffer;
	bind.View = RHIViewDesc::DefaultUAV();
	DXDescriptor* desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(&bind);
	DXDescriptor* CPUDesc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->FindInCacheHeap(&bind);
	uint Values[4] = { 0,0,0,0 };
	GetCommandList()->ClearUnorderedAccessViewUint(desc->GetGPUAddress(), CPUDesc->GetCPUAddress(), DBuffer->GetResource()->GetResource(), Values, 0, nullptr);
}

void D3D12CommandList::CopyResource(RHITexture * Source, RHITexture * Dest)
{

	D3D12RHITexture* DSource = D3D12RHI::DXConv(Source);
	D3D12RHITexture* DDest = D3D12RHI::DXConv(Dest);
	D3D12_RESOURCE_STATES StartState = DSource->GetResource()->GetCurrentState();

	DDest->GetResource()->SetResourceState(this, D3D12_RESOURCE_STATE_COPY_DEST);
	DSource->GetResource()->SetResourceState(this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushBarriers();
	for (int i = 0; i < DSource->GetDescription().Depth; i++)
	{
		int offset = i;
		CD3DX12_TEXTURE_COPY_LOCATION dest(DDest->GetResource()->GetResource(), i);
		CD3DX12_TEXTURE_COPY_LOCATION src(DSource->GetResource()->GetResource(), i);
		CD3DX12_BOX box(0, 0, DSource->GetDescription().Width, DSource->GetDescription().Height);
		GetCommandList()->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
	DSource->GetResource()->SetResourceState(this, StartState);
	FlushBarriers();
}

void D3D12CommandList::CopyResource(RHIBuffer* Source, RHIBuffer* Dest)
{
	//#dxtodo: offset
	D3D12Buffer* DSource = D3D12RHI::DXConv(Source);
	D3D12Buffer* DDest = D3D12RHI::DXConv(Dest);
	D3D12_RESOURCE_STATES StartState = DSource->GetResource()->GetCurrentState();

	DDest->GetResource()->SetResourceState(this, D3D12_RESOURCE_STATE_COPY_DEST);
	DSource->GetResource()->SetResourceState(this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushBarriers();
	GetCommandList()->CopyBufferRegion(DDest->GetResource()->GetResource(), 0, DSource->GetResource()->GetResource(), 0, DSource->GetSize());
	DSource->GetResource()->SetResourceState(this, StartState);
	FlushBarriers();
}

void D3D12CommandList::CopyResource(RHITexture* Source, RHIBuffer* Dest)
{
	D3D12RHITexture* DSource = D3D12RHI::DXConv(Source);
	D3D12Buffer* DDest = D3D12RHI::DXConv(Dest);
	D3D12_RESOURCE_STATES StartState = DSource->GetResource()->GetCurrentState();

	DDest->GetResource()->SetResourceState(this, D3D12_RESOURCE_STATE_COPY_DEST);
	DSource->GetResource()->SetResourceState(this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushBarriers();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;
	for (int i = 0; i < DSource->GetDescription().Depth; i++)
	{
		int offset = i;
		D3D12RHI::DXConv(Device)->GetDevice()->GetCopyableFootprints(&DSource->GetResource()->GetResource()->GetDesc(), 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);
		CD3DX12_TEXTURE_COPY_LOCATION dest(DDest->GetResource()->GetResource(), renderTargetLayout);
		CD3DX12_TEXTURE_COPY_LOCATION src(DSource->GetResource()->GetResource(), i);
		CD3DX12_BOX box(0, 0, DSource->GetDescription().Width, DSource->GetDescription().Height);
		GetCommandList()->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	}
	DSource->GetResource()->SetResourceState(this, StartState);
	FlushBarriers();
}

void D3D12CommandList::SetScissorRect(const RHIScissorRect& rect)
{
	CD3DX12_RECT ScissorR = CD3DX12_RECT(rect.Left, rect.Top, rect.Right, rect.Bottom);
	CurrentCommandList->RSSetScissorRects(1, &ScissorR);
}

void D3D12CommandList::SetDebugMarker(std::string text, uint64 PaletteColour /*= 0*/)
{
#if PIX_ENABLED
	PIXSetMarker(GetCommandList(), PaletteColour, text.c_str());
#endif
}

void D3D12CommandList::PushDebugMarker(std::string text, uint64 PaletteColour /*= 0*/)
{
#if PIX_ENABLED
	PIXBeginEvent(GetCommandList(), PaletteColour, text.c_str());
#endif
}

void D3D12CommandList::PopDebugMarker()
{
#if PIX_ENABLED
	PIXEndEvent(GetCommandList());
#endif
}

void D3D12CommandList::UpdateBufferData(RHIBuffer* Buffer, void * data, size_t length)
{
	D3D12Buffer* DXBuffer = D3D12RHI::DXConv(Buffer);
	DXBuffer->UpdateBufferDataGPU(data, length, this);
}

void D3D12CommandList::ExecuteIndirect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{
	PrepareforDraw();
	ensure(CommandSig != nullptr);
	ensure(ArgumentBuffer);
	ensureMsgf(ArgumentBuffer->GetDesc().UseForExecuteIndirect, "Buffer needs UseForExecuteIndirect flag set");
	ID3D12Resource* counterB = nullptr;
	if (CountBuffer != nullptr)
	{
		counterB = D3D12RHI::DXConv(CountBuffer)->GetResource()->GetResource();
	}
	//PushPrimitiveTopology();
	CurrentCommandList->ExecuteIndirect(CommandSig->GetSigniture(), MaxCommandCount, D3D12RHI::DXConv(ArgumentBuffer)->GetResource()->GetResource(), ArgOffset, counterB, CountBufferOffset);
}

void D3D12CommandList::SetCommandSigniture(RHICommandSignitureDescription desc)
{
	//if (CommandSig->GetDesc() == desc)
	//{
	//	return;
	//}
	EnqueueSafeRHIRelease(CommandSig);
	desc.IsCompute = IsComputeList();
	desc.PSO = CurrentPSO;
	CommandSig = new D3D12CommandSigniture(GetDevice(), desc);
	CommandSig->Build();
	CommandCount++;
}

void D3D12CommandList::SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset)
{
	if (SignitureSlot == -1)
	{
		return;
	}
	if (IsGraphicsList())
	{
		CurrentCommandList->SetGraphicsRoot32BitConstants(SignitureSlot, ValueNum, Data, DataOffset);
	}
	else
	{
		CurrentCommandList->SetComputeRoot32BitConstants(SignitureSlot, ValueNum, Data, DataOffset);
	}
	CommandCount++;
}

ID3D12GraphicsCommandList* D3D12CommandList::GetCommandList()
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
	D3D12RHI::Instance->SetScreenRenderTarget(this);
	D3D12RHI::Instance->RenderToScreen(CurrentCommandList);
}

void D3D12CommandList::ClearScreen()
{
	ensure(ListType == ECommandListType::Graphics);
	ensureMsgf(Device->GetDeviceIndex() == 0, "Only the Primary Device Is allowed to write to the backbuffer");
	D3D12RHI::Instance->ClearRenderTarget(CurrentCommandList);
}

void D3D12CommandList::SetFrameBufferTexture(FrameBuffer * buffer, int slot, const RHIViewDesc & desc)
{
	FlushBarriers();
	ensure(!buffer->IsPendingKill());
	ensure(ListType == ECommandListType::Graphics || ListType == ECommandListType::Compute || ListType == ECommandListType::RayTracing);
	D3D12FrameBuffer* DBuffer = D3D12RHI::DXConv(buffer);
	if (IsComputeList() || IsRaytracingList())
	{
		ensure(DBuffer->IsReadyForCompute());
	}
	else
	{
		StateAssert(DBuffer, EResourceState::PixelShader);
	}
	if (Device->GetStateCache()->RenderTargetCheckAndUpdate(buffer))
	{
		return;
	}
	DBuffer->ValidateState();
	//ensure(DBuffer->CheckDevice(Device->GetDeviceIndex()));
	if (!DBuffer->CheckDevice(Device->GetDeviceIndex()))
	{
		return;
	}
	RootSigniture.SetFrameBufferTexture(slot, buffer, desc);
	CommandCount++;
}
#if WIN10_1903
ID3D12GraphicsCommandList5 * D3D12CommandList::GetCMDList5()
{
	return CmdList5;
}
#endif
void D3D12CommandList::SetDepthBounds(float Min, float Max)
{
	if (!Device->GetCaps().SupportsDepthBoundsTest)
	{
		AD_WARN("SetDepthBounds Is not supported by the device");
		return;
	}
	CommandList1->OMSetDepthBounds(Min, Max);
	CommandCount++;
}
#if WIN10_1809
void D3D12CommandList::TraceRays(const RHIRayDispatchDesc& desc)
{
	PrepareforDraw();
	ensure(CurrentRTState);
	//#DXR: todo
	CurrentRTState->Trace(desc, this, D3D12RHI::DXConv(desc.Target));
	UAVBarrier(desc.Target);
	CommandCount++;
}

void D3D12CommandList::SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct)
{
	ensure(CurrentRTState);
	CurrentRTState->HighLevelStructure = Struct;
}

void D3D12CommandList::SetStateObject(RHIStateObject* Object)
{
	CurrentRTState = D3D12RHI::DXConv(Object);
	CurrentRTState->BindToList(this);
	CommandCount++;
}
#endif

void D3D12CommandList::SetTexture(BaseTextureRef texture, int slot, const RHIViewDesc & desc)
{
	FlushBarriers();
	ensure(texture != nullptr);
	ensure(!texture->IsPendingKill());
	Texture = D3D12RHI::DXConv(texture.Get());
	if (GetDeviceIndex() == 1)
	{
		float t = 0;
	}
	Texture = IRHISharedDeviceObject<BaseTexture>::GetObject<D3D12Texture>(texture.Get(), Device);
	ensureMsgf(Texture->CheckDevice(Device->GetDeviceIndex()), "Attempted to Bind texture that is not on this device");
	if (Device->GetStateCache()->TextureCheckAndUpdate(texture.Get(), slot))
	{
		return;
	}
	Texture->BindToSlot(this, 0);
	Texture->AddRef();
	texture->AddRef();
	//	ensure(Texture->GetResource()->IsValidStateForList(this));
	RootSigniture.SetTexture(slot, Texture, desc);
	CommandCount++;
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot)
{
	if (Slot == -1)
	{
		return;
	}
	FlushBarriers();
	ensure(!buffer->IsPendingKill());
	D3D12Buffer* d3Buffer = D3D12RHI::DXConv(buffer);
	ensure(d3Buffer->CheckDevice(Device->GetDeviceIndex()));
	d3Buffer->SetConstantBufferView(offset, CurrentCommandList, Slot, ListType == ECommandListType::Compute || IsRaytracingList(), Device->GetDeviceIndex());
	CommandCount++;
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, RHIViewDesc Desc, int Slot)
{
	FlushBarriers();
	RootSigniture.SetConstantBufferView(Slot, buffer, 0, Desc);
	CommandCount++;
}

void D3D12CommandList::SetBuffer(RHIBuffer* Buffer, int slot, const RHIViewDesc & desc)
{
	FlushBarriers();
	RootSigniture.SetBufferReadOnly(slot, Buffer, desc);
	CommandCount++;
}

void D3D12CommandList::SetTextureArray(RHITextureArray* array, int slot, const RHIViewDesc& view)
{
	FlushBarriers();
	RootSigniture.SetTextureArray(slot, array, view);
	CommandCount++;
}

void D3D12CommandList::SetUAV(RHIBuffer* buffer, int slot, const RHIViewDesc & view)
{
	FlushBarriers();
	RHIViewDesc v = view;
	v.ViewType = EViewType::UAV;
	RootSigniture.SetUAV(slot, buffer, v);
	CommandCount++;
}

void D3D12CommandList::SetUAV(RHITexture* buffer, int slot, const RHIViewDesc & view)
{
	FlushBarriers();
	RHIViewDesc v = view;
	v.ViewType = EViewType::UAV;
	RootSigniture.SetUAV(slot, buffer, v);
	CommandCount++;
}

void D3D12CommandList::SetUAV(FrameBuffer* buffer, int slot, const RHIViewDesc & view /*= RHIViewDesc()*/)
{
	FlushBarriers();
	RHIViewDesc v = view;
	v.ViewType = EViewType::UAV;
	RootSigniture.SetUAV(slot, buffer, v);
	CommandCount++;
}



D3D12CommandSigniture::D3D12CommandSigniture(DeviceContext * context, RHICommandSignitureDescription desc) :RHICommandSigniture(context, desc)
{

}

ID3D12CommandSignature * D3D12CommandSigniture::GetSigniture()
{
	return CommandSig;
}

void D3D12CommandSigniture::Build()
{
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	ZeroMemory(&commandSignatureDesc, sizeof(commandSignatureDesc));

	D3D12_INDIRECT_ARGUMENT_DESC* argumentDescs = new D3D12_INDIRECT_ARGUMENT_DESC[RHIdesc.ArgumentDescs.size()];
	for (int i = 0; i < RHIdesc.ArgumentDescs.size(); i++)
	{
		argumentDescs[i] = D3D12Helpers::ConvertArg(RHIdesc.ArgumentDescs[i]);
	}
	commandSignatureDesc.pArgumentDescs = argumentDescs;
	commandSignatureDesc.NumArgumentDescs = (UINT)RHIdesc.ArgumentDescs.size();
	commandSignatureDesc.ByteStride = RHIdesc.CommandBufferStide;
	ensure(commandSignatureDesc.ByteStride % 4 == 0);
	ThrowIfFailed(D3D12RHI::DXConv(Context)->GetDevice()->CreateCommandSignature(&commandSignatureDesc, RHIdesc.IsCompute ? nullptr : D3D12RHI::DXConv(RHIdesc.PSO)->RootSig, ID_PASS(&CommandSig)));
	NAME_D3D12_OBJECT(CommandSig);
	delete[] argumentDescs;
}

void D3D12CommandSigniture::Release()
{
	SafeRelease(CommandSig);
}
