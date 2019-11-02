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
#include "DXDescriptor.h"
#include "D3D12RHI.h"
#include "Raytracing/D3D12StateObject.h"
#include "D3D12Buffer.h"
#include "DescriptorCache.h"
#if FORCE_RENDER_PASS_USE
#define CHECKRPASS() ensure(IsInRenderPass);
#else
#define CHECKRPASS() 
#endif

D3D12CommandList::D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType) :RHICommandList(ListType, inDevice)
{
	AddCheckerRef(D3D12CommandList, this);
	mDeviceContext = D3D12RHI::DXConv(inDevice);
	CommandAlloc = new CommandAllocator(ListType, mDeviceContext);
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
	SafeDelete(CommandAlloc);
	//	SafeRelease(CommandSig);
}

bool D3D12CommandList::IsOpen()
{
	return m_IsOpen;
}

void D3D12CommandList::SetPipelineStateDesc(const RHIPipeLineStateDesc& Desc)
{
	if (CurrentPSO != nullptr && CurrentPSO->GetDesc() == Desc)
	{
		return;
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
	CommandAlloc->Reset();
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
	ThrowIfFailed(CurrentCommandList->Reset(GetCommandAllocator(), PSO));
	HandleStallTimer();
	PushState();
	if (ListType != ECommandListType::Copy)
	{
		mDeviceContext->GetHeapManager()->BindHeap(this);
	}
	RootSigniture.Reset();
}
ID3D12CommandAllocator* D3D12CommandList::GetCommandAllocator()
{
#if 0
	if (ListType == ECommandListType::Graphics)
	{
		return D3D12RHI::DXConv(Device)->GetCommandAllocator(ListType);
	}
#endif
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
		CurrentRenderTarget->BindBufferAsRenderTarget(CurrentCommandList, SubResourceIndex);
	}
}

void D3D12CommandList::PrepareforDraw()
{
	SCOPE_CYCLE_COUNTER_GROUP("PrepareforDraw", "RHI");
	PushHeaps();
	//ClearHeaps();
	PushPrimitiveTopology();

	for (int i = 0; i < RootSigniture.GetNumBinds(); i++)
	{
		const RSBind* bind = RootSigniture.GetBind(i);
		
		if (bind->BindType == ERSBindType::Texture && bind->IsBound())
		{
			DXDescriptor* desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
			if (IsGraphicsList())
			{
				//TextureResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				GetCommandList()->SetGraphicsRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
			else
			{
				GetCommandList()->SetComputeRootDescriptorTable(bind->BindParm->SignitureSlot, desc->GetGPUAddress());
			}
		}
		else if (bind->BindType == ERSBindType::FrameBuffer && bind->IsBound())
		{
			DXDescriptor* desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
			if (IsGraphicsList())
			{
				//TextureResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
			DXDescriptor* desc = nullptr;
			//check(bind->IsBound());
			if (bind->IsBound())
			{
				desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
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
//			ensure(bind->View.ViewType != EViewType::Limit);
			DXDescriptor* desc = nullptr;
			//check(bind->IsBound());
			if (bind->IsBound())
			{
				desc = D3D12RHI::DXConv(Device)->GetDescriptorCache()->GetOrCreate(bind);
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
		else if (bind->BindType == ERSBindType::UAV)
		{
			
			DXDescriptor* desc = nullptr;		
			//check(bind->IsBound());
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
}


void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	PrepareforDraw();
	CHECKRPASS();
	ensure(m_IsOpen);
	ensure(ListType == ECommandListType::Graphics);
	CurrentCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
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
		ThrowIfFailed(CurrentCommandList->Close());
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
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_DIRECT, GetCommandAllocator(), PSO, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Compute || IsRaytracingList())
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_COMPUTE, GetCommandAllocator(), PSO, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
	else if (ListType == ECommandListType::Copy)
	{
		ThrowIfFailed(mDeviceContext->GetDevice()->CreateCommandList(Device->GetNodeMask(), D3D12_COMMAND_LIST_TYPE_COPY, GetCommandAllocator(), nullptr, IID_PPV_ARGS(&CurrentCommandList)));
		ThrowIfFailed(CurrentCommandList->Close());
	}
#if WIN10_1809
	if (mDeviceContext->SupportsCommandList4())
	{
		CurrentCommandList->QueryInterface(IID_PPV_ARGS(&CurrentADVCommandList));
	}
#endif
	CurrentCommandList->QueryInterface(IID_PPV_ARGS(&CommandList1));
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
	PrepareforDraw();
	ensure(ListType == ECommandListType::Compute);
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
	D3D12RHI::DXConv(buffer)->ClearBuffer(CurrentCommandList);
}

void D3D12CommandList::UAVBarrier(FrameBuffer* target)
{
	D3D12FrameBuffer* dtarget = D3D12RHI::DXConv(target);
	CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->GetResource(0)->GetResource()));
	//CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->UAVCounter));
}

void D3D12CommandList::UAVBarrier(RHIBuffer* target)
{
	D3D12Buffer* dtarget = D3D12RHI::DXConv(target);
	CurrentCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->GetResource()->GetResource()));
}

RHIRootSigniture * D3D12CommandList::GetRootSig()
{
	return &RootSigniture;
}

void D3D12CommandList::ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{
	PrepareforDraw();
	ensure(CommandSig != nullptr);
	ensure(ArgumentBuffer);
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
	ensure(ListType == ECommandListType::Graphics || ListType == ECommandListType::Compute || ListType == ECommandListType::RayTracing);
	D3D12FrameBuffer* DBuffer = D3D12RHI::DXConv(buffer);
	if (IsComputeList() || IsRaytracingList())
	{
		ensure(DBuffer->IsReadyForCompute());
	}
	if (Device->GetStateCache()->RenderTargetCheckAndUpdate(buffer))
	{
		return;
	}
	ensure(DBuffer->CheckDevice(Device->GetDeviceIndex()));
#if DESC_CREATE
	RHIViewDesc d;
	d.Resource = Resourceindex;
	d.ViewType = EViewType::SRV;
	RootSigniture.SetFrameBufferTexture(slot, buffer, Resourceindex, d);
#else
	DBuffer->BindBufferToTexture(CurrentCommandList, slot, Resourceindex, Device, (ListType == ECommandListType::Compute || IsRaytracingList()));
#endif
}

void D3D12CommandList::BindSRV(FrameBuffer* Buffer, int slot, RHIViewDesc Desc)
{
	//D3D12FrameBuffer* DBuffer = D3D12RHI::DXConv(Buffer);
	//DBuffer->BindSRV(this, slot, Desc);
}

void D3D12CommandList::SetDepthBounds(float Min, float Max)
{
	if (!Device->GetCaps().SupportsDepthBoundsTest)
	{
		return;
	}
	CommandList1->OMSetDepthBounds(Min, Max);
}
#if WIN10_1809
void D3D12CommandList::TraceRays(const RHIRayDispatchDesc& desc)
{
	PrepareforDraw();
	ensure(CurrentRTState);
	//#DXR: todo
	CurrentRTState->Trace(desc, this, D3D12RHI::DXConv(desc.Target));
	UAVBarrier(desc.Target);
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
}
#endif
void D3D12CommandList::SetTexture(BaseTextureRef texture, int slot, const RHIViewDesc & desc)
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
	RootSigniture.SetTexture(slot, texture,desc);
#if !DESC_CREATE
	if (CurrentCommandList != nullptr)
	{
		Texture->BindToSlot(this, slot);
	}
#endif
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot)
{
	if (Slot == -1)
	{
		return;
	}
	ensure(!buffer->IsPendingKill());
	D3D12Buffer* d3Buffer = D3D12RHI::DXConv(buffer);
	ensure(d3Buffer->CheckDevice(Device->GetDeviceIndex()));
	d3Buffer->SetConstantBufferView(offset, CurrentCommandList, Slot, ListType == ECommandListType::Compute || IsRaytracingList(), Device->GetDeviceIndex());
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, RHIViewDesc Desc, int Slot)
{
	RootSigniture.SetConstantBufferView(Slot, buffer, 0, Desc);
}

void D3D12CommandList::SetBuffer(RHIBuffer* Buffer, int slot, const RHIViewDesc & desc)
{
	RootSigniture.SetBufferReadOnly(slot, Buffer, desc);
}

void D3D12CommandList::SetTextureArray(RHITextureArray* array, int slot, const RHIViewDesc& view)
{
	RootSigniture.SetTextureArray(slot, array, view);
}

void D3D12CommandList::SetUAV(RHIBuffer* buffer, int slot, const RHIViewDesc & view)
{
	RHIViewDesc v = view;
	v.ViewType = EViewType::UAV;
	RootSigniture.SetUAV(slot, buffer, v);
}


void D3D12CommandList::SetUAV(FrameBuffer* buffer, int slot, const RHIViewDesc & view /*= RHIViewDesc()*/)
{
	RHIViewDesc v = view;
	v.ViewType = EViewType::UAV;
	RootSigniture.SetUAV(slot, buffer, v);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Texture Array
D3D12RHITextureArray::D3D12RHITextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
{
	AddCheckerRef(D3D12RHITextureArray, this);
	Device = D3D12RHI::DXConv(device);
}

D3D12RHITextureArray::~D3D12RHITextureArray()
{}

//Add a frame buffer to this heap and ask it to create one in our heap
void D3D12RHITextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{
	ensure(!Buffer->IsPendingKill());
	D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(Buffer);
	ensure(dBuffer->CheckDevice(Device->GetDeviceIndex()));
	if (slot < LinkedBuffers.size())
	{
		LinkedBuffers[slot] = dBuffer;
	}
	else
	{
		LinkedBuffers.push_back(dBuffer);
	}
}

void D3D12RHITextureArray::BindToShader(RHICommandList * list, int slot)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(list);
	ensure(DXList->GetDevice() == Device);
	DXList->SetTextureArray(this, slot, RHIViewDesc());
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
	if (TargetIndex < LinkedBuffers.size())
	{
		LinkedBuffers[TargetIndex] = nullptr;
	}
}

void D3D12RHITextureArray::SetFrameBufferFormat(const RHIFrameBufferDesc & desc)
{
	NullHeapDesc = D3D12FrameBuffer::GetSrvDesc(0, desc);
}

DXDescriptor * D3D12RHITextureArray::GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap)
{
	if (heap == nullptr)
	{
		heap = Device->GetHeapManager()->GetMainHeap();
	}
	DXDescriptor* Descriptor = heap->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumEntries);
	for (int i = 0; i < NumEntries; i++)
	{
		if (i < LinkedBuffers.size() && LinkedBuffers[i] != nullptr)
		{
			D3D12FrameBuffer* dBuffer = D3D12RHI::DXConv(LinkedBuffers[i]);
			ensure(dBuffer->CheckDevice(Device->GetDeviceIndex()));
			dBuffer->CreateSRVInHeap(i, Descriptor);
		}
		else
		{
			Descriptor->CreateShaderResourceView(nullptr, &NullHeapDesc, i);
		}
	}
	Descriptor->Recreate();
	return Descriptor;
}

uint64 D3D12RHITextureArray::GetHash()
{
	uint64 hash = 0;
	for (int i = 0; i < LinkedBuffers.size(); i++)
	{
		HashUtils::hash_combine(hash, LinkedBuffers[i]);
	}
	return hash;
}

void D3D12RHITextureArray::Release()
{
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12RHITextureArray, this);
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
	ThrowIfFailed(D3D12RHI::DXConv(Context)->GetDevice()->CreateCommandSignature(&commandSignatureDesc, RHIdesc.IsCompute ? nullptr : D3D12RHI::DXConv(RHIdesc.PSO)->RootSig, IID_PPV_ARGS(&CommandSig)));
	NAME_D3D12_OBJECT(CommandSig);
	delete[] argumentDescs;
}

void D3D12CommandSigniture::Release()
{
	SafeRelease(CommandSig);
}
