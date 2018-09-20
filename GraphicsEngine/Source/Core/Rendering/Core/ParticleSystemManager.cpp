#include "Stdafx.h"
#include "ParticleSystemManager.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Shaders/Particle/Shader_ParticleCompute.h"
#include "Rendering/Shaders/Particle/Shader_ParticleDraw.h"
ParticleSystemManager* ParticleSystemManager::Instance = nullptr;

ParticleSystemManager::ParticleSystemManager()
{
	Init();
}

ParticleSystemManager::~ParticleSystemManager()
{}

void ParticleSystemManager::Init()
{
	GPU_ParticleData = RHI::CreateRHIBuffer(RHIBuffer::BufferType::GPU);
	RHIBufferDesc desc;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.Stride = sizeof(ParticleData);
	desc.ElementCount = CurrentParticleCount;
	desc.AllowUnorderedAccess = true;

	GPU_ParticleData->CreateBuffer(desc);
	GPU_ParticleData_UAV = RHI::CreateUAV();
	GPU_ParticleData_UAV->CreateUAVFromRHIBuffer(GPU_ParticleData);

	CounterBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::GPU);
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(Counters);
	desc.ElementCount = 1;
	CounterBuffer->CreateBuffer(desc);
	Counters count = Counters();
	count.deadCount = CurrentParticleCount;
	CounterBuffer->UpdateIndexBuffer(&count, sizeof(Counters));
	CounterUAV = RHI::CreateUAV();
	CounterUAV->CreateUAVFromRHIBuffer(CounterBuffer);
	CmdList = RHI::CreateCommandList(ECommandListType::Compute);
	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleCompute>());
	SetupCommandBuffer();


	AliveParticleIndexs = RHI::CreateRHIBuffer(RHIBuffer::BufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(UINT);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.ElementCount = CurrentParticleCount;
	AliveParticleIndexs->CreateBuffer(desc);
	AliveParticleIndexs_UAV = RHI::CreateUAV();
	AliveParticleIndexs_UAV->CreateUAVFromRHIBuffer(AliveParticleIndexs);
	//AliveParticleIndexs->UpdateBufferData(0, 0, EBufferResourceState::UnorderedAccess);
	DeadParticleIndexs = RHI::CreateRHIBuffer(RHIBuffer::BufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(UINT);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::Static;
	desc.ElementCount = CurrentParticleCount;
	DeadParticleIndexs->CreateBuffer(desc);
	uint32_t* indices = new uint32_t[MAX_PARTICLES];
	for (uint32_t i = 0; i < MAX_PARTICLES; ++i)
	{
		indices[i] = i;
	}
	DeadParticleIndexs->UpdateBufferData(indices, MAX_PARTICLES, EBufferResourceState::UnorderedAccess);
	DeadParticleIndexs_UAV = RHI::CreateUAV();
	DeadParticleIndexs_UAV->CreateUAVFromRHIBuffer(DeadParticleIndexs);
	delete[] indices;
}

void ParticleSystemManager::SetupCommandBuffer()
{
	DispatchCommandBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::GPU);
	RHIBufferDesc desc;
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(IndrectDispatchArgs) * 2;
	desc.ElementCount = 1;
	desc.AllowUnorderedAccess = true;
	DispatchCommandBuffer->CreateBuffer(desc);
	IndrectDispatchArgs Data[2];
	Data[0].dispatchArgs = D3D12_DISPATCH_ARGUMENTS();
	Data[0].dispatchArgs.ThreadGroupCountX = 1;
	Data[0].dispatchArgs.ThreadGroupCountY = 1;
	Data[0].dispatchArgs.ThreadGroupCountZ = 1;
	Data[1].dispatchArgs = D3D12_DISPATCH_ARGUMENTS();
	Data[1].dispatchArgs.ThreadGroupCountX = 1;
	Data[1].dispatchArgs.ThreadGroupCountY = 1;
	Data[1].dispatchArgs.ThreadGroupCountZ = 1;
	DispatchCommandBuffer->UpdateBufferData(Data, sizeof(IndrectDispatchArgs) * 2, EBufferResourceState::UnorderedAccess);
	DispatchCommandBuffer_UAV = RHI::CreateUAV();
	DispatchCommandBuffer_UAV->CreateUAVFromRHIBuffer(DispatchCommandBuffer);
	CmdList->SetUpCommandSigniture(sizeof(IndrectDispatchArgs), true);
	RenderCommandBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::GPU);
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(IndirectArgs);
	desc.ElementCount = CurrentParticleCount;
	desc.AllowUnorderedAccess = true;
	RenderCommandBuffer->CreateBuffer(desc);
	std::vector<IndirectArgs> cmdbuffer;
	cmdbuffer.resize(CurrentParticleCount);
	for (int i = 0; i < cmdbuffer.size(); i++)
	{
		cmdbuffer[i].drawArguments.VertexCountPerInstance = 6;
		cmdbuffer[i].drawArguments.InstanceCount = 1;
		cmdbuffer[i].drawArguments.StartVertexLocation = 0;
		cmdbuffer[i].drawArguments.StartInstanceLocation = 0;
		cmdbuffer[i].data = 0;
	}
	RenderCommandBuffer->UpdateBufferData(cmdbuffer.data(), cmdbuffer.size(), EBufferResourceState::UnorderedAccess);
	RenderCommandBuffer_UAV = RHI::CreateUAV();
	RenderCommandBuffer_UAV->CreateUAVFromRHIBuffer(RenderCommandBuffer);
	RenderList = RHI::CreateCommandList();
	PipeLineState pls = PipeLineState();
	pls.Cull = false;
	pls.RenderTargetDesc = RHIPipeRenderTargetDesc();
	pls.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	pls.RenderTargetDesc.NumRenderTargets = 1;
	pls.RenderTargetDesc.DSVFormat = FORMAT_D32_FLOAT;
	RenderList->SetPipelineState(pls);
	RenderList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleDraw>());
	RenderList->SetUpCommandSigniture(sizeof(IndirectArgs), false);


	float g_quad_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,0.0f,
	1.0f, -1.0f, 0.0f,0.0f,
	-1.0f,  1.0f, 0.0f,0.0f,
	-1.0f,  1.0f, 0.0f,0.0f,
	1.0f, -1.0f, 0.0f,0.0f,
	1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
	ParticleRenderConstants = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	ParticleRenderConstants->CreateConstantBuffer(sizeof(RenderData), 1);
}

void ParticleSystemManager::PreRenderUpdate(Camera* c)
{
	RenderData.CameraUp_worldspace = c->GetUp();
	RenderData.CameraRight_worldspace = c->GetRight();
	RenderData.VPMat = c->GetViewProjection();
	ParticleRenderConstants->UpdateConstantBuffer(&RenderData, 0);
}

ParticleSystemManager * ParticleSystemManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ParticleSystemManager();
	}
	return Instance;
}

void ParticleSystemManager::ShutDown()
{
	EnqueueSafeRHIRelease(GPU_ParticleData);
	EnqueueSafeRHIRelease(GPU_ParticleData_UAV);
	EnqueueSafeRHIRelease(CmdList);
	EnqueueSafeRHIRelease(RenderCommandBuffer);
	EnqueueSafeRHIRelease(RenderList);
	EnqueueSafeRHIRelease(VertexBuffer);
	EnqueueSafeRHIRelease(ParticleRenderConstants);
	EnqueueSafeRHIRelease(CounterBuffer);
	EnqueueSafeRHIRelease(CounterUAV);
	EnqueueSafeRHIRelease(DispatchCommandBuffer);
	EnqueueSafeRHIRelease(DispatchCommandBuffer_UAV);
	EnqueueSafeRHIRelease(AliveParticleIndexs);
	EnqueueSafeRHIRelease(RenderCommandBuffer_UAV);
	EnqueueSafeRHIRelease(DeadParticleIndexs);
	EnqueueSafeRHIRelease(AliveParticleIndexs_UAV);
	EnqueueSafeRHIRelease(DeadParticleIndexs_UAV);
}

void ParticleSystemManager::Simulate()
{
	CmdList->ResetList();
	DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::UnorderedAccess);
	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_StartSimulation>());
	CounterUAV->Bind(CmdList, 0);
	DispatchCommandBuffer_UAV->Bind(CmdList, 1);
	CmdList->Dispatch(1, 1, 1);
	DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);

	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleEmit>());
	CounterUAV->Bind(CmdList, 1);
	GPU_ParticleData_UAV->Bind(CmdList, 0);
	AliveParticleIndexs_UAV->Bind(CmdList, 2);
	DeadParticleIndexs_UAV->Bind(CmdList, 3);
	emitcount++;
	int on = (emitcount % 10 == 0);
	CmdList->SetRootConstant(4, 1, &on, 0);
	CmdList->ExecuteIndiect(1, DispatchCommandBuffer, 0, nullptr, 0);

	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleCompute>());
	GPU_ParticleData_UAV->Bind(CmdList, 0);
	CounterUAV->Bind(CmdList, 1);
	AliveParticleIndexs->BindBufferReadOnly(CmdList, 2);
	DeadParticleIndexs_UAV->Bind(CmdList, 3);
	CmdList->ExecuteIndiect(1, DispatchCommandBuffer, 0, nullptr, 0);

	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_EndSimulation>());

	RenderCommandBuffer_UAV->Bind(CmdList, 1);
	//AliveParticleIndexs->SetBufferState(CmdList,EBufferResourceState::)
	AliveParticleIndexs->BindBufferReadOnly(CmdList, 0);
	CmdList->ExecuteIndiect(1, DispatchCommandBuffer, sizeof(IndrectDispatchArgs), nullptr, 0);
	RenderCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);
	CmdList->Execute();
}

void ParticleSystemManager::Render(FrameBuffer* BufferTarget)
{

	RenderList->ResetList();
	//RenderList->CreatePipelineState(ShaderComplier::GetShader<Shader_ParticleDraw>(), BufferTarget);
	RenderList->SetRenderTarget(BufferTarget);
	RenderList->SetVertexBuffer(VertexBuffer);
	RenderList->SetConstantBufferView(ParticleRenderConstants, 0, 2);
	GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::Read);
	GPU_ParticleData->BindBufferReadOnly(RenderList, 1);
	RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::IndirectArgs);
	RenderList->ExecuteIndiect(MAX_PARTICLES, RenderCommandBuffer, 0, nullptr, 0);
	GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::UnorderedAccess);
	RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::UnorderedAccess);
	RenderList->Execute();
}