#include "Stdafx.h"
#include "ParticleSystemManager.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Shaders/Particle/Shader_ParticleCompute.h"
#include "Rendering/Shaders/Particle/Shader_ParticleDraw.h"
#include "RHI/DeviceContext.h"
#include "Core/Assets/AssetManager.h"
ParticleSystemManager* ParticleSystemManager::Instance = nullptr;

ParticleSystemManager::ParticleSystemManager()
{
	Init();
}

ParticleSystemManager::~ParticleSystemManager()
{}

void ParticleSystemManager::Init()
{
	GPU_ParticleData = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	RHIBufferDesc desc;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.Stride = sizeof(ParticleData);
	desc.ElementCount = CurrentParticleCount;
	desc.AllowUnorderedAccess = true;
	desc.CreateUAV = true;
	GPU_ParticleData->SetDebugName("Particle Data");
	GPU_ParticleData->CreateBuffer(desc);


	CounterBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(Counters);
	desc.ElementCount = 1;
	CounterBuffer->CreateBuffer(desc);
	Counters count = Counters();
	count.deadCount = CurrentParticleCount;
	CounterBuffer->UpdateIndexBuffer(&count, sizeof(Counters));
	CounterBuffer->SetDebugName("Counter");
	CounterUAV = RHI::CreateUAV();
	CounterUAV->CreateUAVFromRHIBuffer(CounterBuffer);
	CmdList = RHI::CreateCommandList(ECommandListType::Compute);
	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleCompute>());
	SetupCommandBuffer();


	AliveParticleIndexs = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(unsigned int);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.ElementCount = CurrentParticleCount;
	desc.CreateUAV = true;
	AliveParticleIndexs->CreateBuffer(desc);
	AliveParticleIndexs->SetDebugName("Alive Pre sim");


	AliveParticleIndexs_PostSim = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(unsigned int);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.ElementCount = CurrentParticleCount;
	desc.CreateUAV = true;
	AliveParticleIndexs_PostSim->CreateBuffer(desc);
	AliveParticleIndexs_PostSim->SetDebugName("Alive Post Sim");
	AliveParticleIndexs_PostSim_UAV = RHI::CreateUAV();
	AliveParticleIndexs_PostSim_UAV->CreateUAVFromRHIBuffer(AliveParticleIndexs_PostSim);

	DeadParticleIndexs = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(unsigned int);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::Static;
	desc.ElementCount = CurrentParticleCount;
	DeadParticleIndexs->SetDebugName("Dead Particles");
	DeadParticleIndexs->CreateBuffer(desc);
	uint32_t* indices = new uint32_t[MAX_PARTICLES];
	for (uint32_t i = 0; i < MAX_PARTICLES; ++i)
	{
		indices[i] = i;
	}
	DeadParticleIndexs->UpdateBufferData(indices, MAX_PARTICLES, EBufferResourceState::UnorderedAccess);
	DeadParticleIndexs->SetDebugName("Dead particle indexes");
	DeadParticleIndexs_UAV = RHI::CreateUAV();
	DeadParticleIndexs_UAV->CreateUAVFromRHIBuffer(DeadParticleIndexs);
	delete[] indices;

	//test
	TEstTex = AssetManager::DirectLoadTextureAsset("texture\\smoke.png", true);
}

void ParticleSystemManager::SetupCommandBuffer()
{
	DispatchCommandBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	RHIBufferDesc desc;
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(IndirectDispatchArgs) * 2;
	desc.ElementCount = 1;
	desc.AllowUnorderedAccess = true;
	DispatchCommandBuffer->CreateBuffer(desc);
	DispatchArgs Data[2];
	Data[0].dispatchArgs = IndirectDispatchArgs();
	Data[0].dispatchArgs.ThreadGroupCountX = 1;
	Data[0].dispatchArgs.ThreadGroupCountY = 1;
	Data[0].dispatchArgs.ThreadGroupCountZ = 1;
	Data[1].dispatchArgs = IndirectDispatchArgs();
	Data[1].dispatchArgs.ThreadGroupCountX = 1;
	Data[1].dispatchArgs.ThreadGroupCountY = 1;
	Data[1].dispatchArgs.ThreadGroupCountZ = 1;
	DispatchCommandBuffer->UpdateBufferData(Data, sizeof(IndirectDispatchArgs) * 2, EBufferResourceState::UnorderedAccess);
	DispatchCommandBuffer->SetDebugName("Dispatch Command Buffer");
	DispatchCommandBuffer_UAV = RHI::CreateUAV();
	DispatchCommandBuffer_UAV->CreateUAVFromRHIBuffer(DispatchCommandBuffer);
#if USE_INDIRECTCOMPUTE
	CmdList->SetUpCommandSigniture(sizeof(DispatchArgs), true);
#endif
	RenderCommandBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
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
	RenderCommandBuffer->SetDebugName("RenderCommandBuffer");
	RenderCommandBuffer_UAV = RHI::CreateUAV();
	RenderCommandBuffer_UAV->CreateUAVFromRHIBuffer(RenderCommandBuffer);

	RenderList = RHI::CreateCommandList();
	PipeLineState pls = PipeLineState();
	pls.Cull = false;
	pls.RenderTargetDesc = RHIPipeRenderTargetDesc();
	pls.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	pls.RenderTargetDesc.NumRenderTargets = 1;
	pls.RenderTargetDesc.DSVFormat = FORMAT_D32_FLOAT;
	pls.Blending = true;
	RenderList->SetPipelineState(pls);
	RenderList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleDraw>());
#if 1//USE_INDIRECTCOMPUTE
	RenderList->SetUpCommandSigniture(sizeof(IndirectArgs), false);
#endif


	float g_quad_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,0.0f,
	1.0f, -1.0f, 0.0f,0.0f,
	-1.0f,  1.0f, 0.0f,0.0f,
	-1.0f,  1.0f, 0.0f,0.0f,
	1.0f, -1.0f, 0.0f,0.0f,
	1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
	ParticleRenderConstants = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	ParticleRenderConstants->CreateConstantBuffer(sizeof(RenderData), 1);
}

void ParticleSystemManager::PreRenderUpdate(Camera* c)
{
	RenderData.CameraUp_worldspace = glm::vec4(c->GetUp(), 1.0f);
	RenderData.CameraRight_worldspace = glm::vec4(c->GetRight(), 1.0f);
	RenderData.VPMat = c->GetViewProjection();
	if (ParticleRenderConstants)
	{
		ParticleRenderConstants->UpdateConstantBuffer(&RenderData, 0);
	}
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
	EnqueueSafeRHIRelease(DeadParticleIndexs_UAV);
	EnqueueSafeRHIRelease(TEstTex);
	EnqueueSafeRHIRelease(AliveParticleIndexs_PostSim);
	EnqueueSafeRHIRelease(AliveParticleIndexs_PostSim_UAV);
}

void ParticleSystemManager::Simulate()
{
	//return;
	CmdList->ResetList();
	CmdList->StartTimer(EGPUTIMERS::ParticleSimulation);
	DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::UnorderedAccess);
	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_StartSimulation>());
	CounterUAV->Bind(CmdList, 0);
	DispatchCommandBuffer_UAV->Bind(CmdList, 1);
	emitcount++;
	int on = 1;// (emitcount % 10 == 0);
	CmdList->SetRootConstant(2, 1, &on, 0);
	CmdList->Dispatch(1, 1, 1);
	DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);

	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleEmit>());
	CounterUAV->Bind(CmdList, 1);
	GPU_ParticleData->GetUAV()->Bind(CmdList, 0);
	AliveParticleIndexs->GetUAV()->Bind(CmdList, 2);

	DeadParticleIndexs_UAV->Bind(CmdList, 3);
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndiect(1, DispatchCommandBuffer, 0, nullptr, 0);
#else
	CmdList->Dispatch(MAX_PARTICLES, 1, 1);
#endif

	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleCompute>());
	GPU_ParticleData->GetUAV()->Bind(CmdList, 0);
	CounterUAV->Bind(CmdList, 1);
	AliveParticleIndexs->BindBufferReadOnly(CmdList, 2);
	DeadParticleIndexs_UAV->Bind(CmdList, 3);
	AliveParticleIndexs_PostSim_UAV->Bind(CmdList, 4);
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndiect(1, DispatchCommandBuffer, sizeof(DispatchArgs), nullptr, 0);
#else
	CmdList->Dispatch(MAX_PARTICLES, 1, 1);
#endif

	CmdList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_EndSimulation>());
	AliveParticleIndexs_PostSim->BindBufferReadOnly(CmdList, 0);
	RenderCommandBuffer_UAV->Bind(CmdList, 1);
	CounterUAV->Bind(CmdList, 2);
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndiect(1, DispatchCommandBuffer, sizeof(DispatchArgs), nullptr, 0);
#else
	CmdList->Dispatch(MAX_PARTICLES, 1, 1);
#endif
	RenderCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);
	CmdList->EndTimer(EGPUTIMERS::ParticleSimulation);
	CmdList->Execute();
	CmdList->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void ParticleSystemManager::Render(FrameBuffer* BufferTarget)
{
	//return;
	RenderList->ResetList();
	RenderList->StartTimer(EGPUTIMERS::ParticleDraw);
	RenderList->SetPipelineStateObject(ShaderComplier::GetShader<Shader_ParticleDraw>(), BufferTarget);
	RenderList->SetRenderTarget(BufferTarget);
	RenderList->SetVertexBuffer(VertexBuffer);
	RenderList->SetConstantBufferView(ParticleRenderConstants, 0, 2);
	GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::Read);
	GPU_ParticleData->BindBufferReadOnly(RenderList, 1);
	RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::IndirectArgs);
	RenderList->SetTexture(TEstTex, 3);
	RenderList->ExecuteIndiect(MAX_PARTICLES, RenderCommandBuffer, 0, nullptr, 0);
	GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::UnorderedAccess);
	RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::UnorderedAccess);
	RenderList->EndTimer(EGPUTIMERS::ParticleDraw);
	RenderList->Execute();
	CmdList->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
}