
#include "ParticleSystemManager.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Shaders/Particle/Shader_ParticleCompute.h"
#include "Rendering/Shaders/Particle/Shader_ParticleDraw.h"
#include "RHI/DeviceContext.h"
#include "Core/Assets/AssetManager.h"
#include "GPUParticleSystem.h"
#include "Core/Utils/MemoryUtils.h"

static ConsoleVariable PauseVar("PS.PauseSim", 0, ECVarType::ConsoleOnly);

ParticleSystemManager* ParticleSystemManager::Instance = nullptr;

ParticleSystemManager::ParticleSystemManager()
{
	if (!RHI::GetRenderSettings()->EnableGPUParticles)
	{
		return;
	}
	InitCommon();
	Testsystem = new ParticleSystem();
	Testsystem->Init();

	AddSystem(Testsystem);
	Testsystem = new ParticleSystem();

	Testsystem->EmitCountPerSecond = 10;
	Testsystem->ParticleTexture = AssetManager::DirectLoadTextureAsset("texture\\billboardgrass0002.png", false);
	//Testsystem->ParticleTexture->AddRef();
	Testsystem->Init();
	AddSystem(Testsystem);
}

ParticleSystemManager::~ParticleSystemManager()
{

}

void ParticleSystemManager::InitCommon()
{
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
	ParticleRenderConstants->CreateConstantBuffer(sizeof(RenderData), 2);

	CmdList = RHI::CreateCommandList(ECommandListType::Compute);

#if USE_INDIRECTCOMPUTE
	RHICommandSignitureDescription sigdesc;
	sigdesc.ArgumentDescs.resize(1);
	sigdesc.ArgumentDescs[0].Type = INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DISPATCH;
	sigdesc.CommandBufferStide = sizeof(DispatchArgs);
	CmdList->SetCommandSigniture(sigdesc);
#endif

	RenderList = RHI::CreateCommandList();
	RHIPipeLineStateDesc pdesc;
	pdesc.Cull = false;
	pdesc.RenderTargetDesc = RHIPipeRenderTargetDesc();
	pdesc.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	pdesc.RenderTargetDesc.NumRenderTargets = 1;
	pdesc.RenderTargetDesc.DSVFormat = FORMAT_D32_FLOAT;
	pdesc.Blending = false;
	pdesc.ShaderInUse = ShaderComplier::GetShader<Shader_ParticleDraw>();
	RenderList->SetPipelineStateDesc(pdesc);
#if USE_INDIRECTCOMPUTE
	sigdesc = RHICommandSignitureDescription();
	sigdesc.ArgumentDescs.resize(2);
	sigdesc.ArgumentDescs[0].Type = INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_CONSTANT;
	sigdesc.ArgumentDescs[0].Constant.RootParameterIndex = 0;
	sigdesc.ArgumentDescs[0].Constant.Num32BitValuesToSet = 1;
	sigdesc.ArgumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
	sigdesc.ArgumentDescs[1].Type = INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DRAW;
	sigdesc.CommandBufferStide = sizeof(IndirectArgs);
	RenderList->SetCommandSigniture(sigdesc);
#endif
}


void ParticleSystemManager::PreRenderUpdate(Camera* c)
{
	for (int i = 0; i < EEye::Limit; i++)
	{
		RenderData[i].CameraUp_worldspace = glm::vec4(c->GetUp(), 1.0f);
		RenderData[i].CameraRight_worldspace = glm::vec4(c->GetRight(), 1.0f);
		RenderData[i].VPMat = c->GetViewProjection();
		if (ParticleRenderConstants)
		{
			ParticleRenderConstants->UpdateConstantBuffer(&RenderData, i);
		}
	}
	float time = Engine::GetDeltaTime();
	for (int i = 0; i < ParticleSystems.size(); i++)
	{
		ParticleSystems[i]->Tick(time);
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
	if (Instance != nullptr)
	{
		Instance->ShutDown_I();
	}
}

void ParticleSystemManager::ShutDown_I()
{
	for (int i = 0; i < ParticleSystems.size(); i++)
	{
		ParticleSystems[i]->Release();
		SafeDelete(ParticleSystems[i]);
	}
	EnqueueSafeRHIRelease(CmdList);
	EnqueueSafeRHIRelease(RenderList);
	EnqueueSafeRHIRelease(VertexBuffer);
	EnqueueSafeRHIRelease(ParticleRenderConstants);
	SafeDelete(Instance);
}

void ParticleSystemManager::Sync(ParticleSystem* system)
{
	CmdList->UAVBarrier(system->AliveParticleIndexs->GetUAV());
	CmdList->UAVBarrier(system->DeadParticleIndexs->GetUAV());
	CmdList->UAVBarrier(system->CounterBuffer->GetUAV());
	CmdList->UAVBarrier(system->DispatchCommandBuffer->GetUAV());
	CmdList->UAVBarrier(system->AliveParticleIndexs_PostSim->GetUAV());
	CmdList->UAVBarrier(system->GPU_ParticleData->GetUAV());
}

void ParticleSystemManager::SimulateSystem(ParticleSystem * System)
{
	if (!System->ShouldSimulate || PauseVar.GetBoolValue())
	{
		return;
	}
	System->DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::UnorderedAccess);
	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_StartSimulation>()));
	System->CounterBuffer->GetUAV()->Bind(CmdList, 0);
	System->DispatchCommandBuffer->GetUAV()->Bind(CmdList, 1);
	CmdList->SetRootConstant(2, 1, &System->RealEmissionCount, 0);
	CmdList->Dispatch(1, 1, 1);
	Sync(System);
	System->DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);

	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(System->EmitShader));
	System->CounterBuffer->GetUAV()->Bind(CmdList, 1);
	System->GPU_ParticleData->GetUAV()->Bind(CmdList, 0);
	System->GetPreSimList()->GetUAV()->Bind(CmdList, 2);

	System->DeadParticleIndexs->GetUAV()->Bind(CmdList, 3);
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndiect(1, System->DispatchCommandBuffer, 0, nullptr, 0);
#else
	CmdList->Dispatch(System->MaxParticleCount, 1, 1);
#endif
	Sync(System);
	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(System->SimulateShader));
	float DT = Engine::GetDeltaTime();
	CmdList->SetRootConstant(System->SimulateShader->GetSlotForName("emitData"), 1, &DT, 0);
	CmdList->SetUAV(System->GPU_ParticleData->GetUAV(), "newPosVelo");
	CmdList->SetUAV(System->CounterBuffer->GetUAV(), "CounterBuffer");
	System->GetPreSimList()->BindBufferReadOnly(CmdList, System->SimulateShader->GetSlotForName("AliveIndexs"));
	System->DeadParticleIndexs->GetUAV()->Bind(CmdList, System->SimulateShader->GetSlotForName("DeadIndexs"));
	System->GetPostSimList()->GetUAV()->Bind(CmdList, System->SimulateShader->GetSlotForName("PostSim_AliveIndex"));
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndiect(1, System->DispatchCommandBuffer, sizeof(DispatchArgs), nullptr, 0);
#else
	CmdList->Dispatch(System->MaxParticleCount, 1, 1);
#endif
	Sync(System);
	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_EndSimulation>()));
#if 0
	System->GetPostSimList()->BindBufferReadOnly(CmdList, 2);
	System->RenderCommandBuffer->GetUAV()->Bind(CmdList, 0);
	System->CounterBuffer->GetUAV()->Bind(CmdList, 1);
#else
	System->GetPostSimList()->BindBufferReadOnly(CmdList, 0);
	System->RenderCommandBuffer->GetUAV()->Bind(CmdList, 1);
	System->CounterBuffer->GetUAV()->Bind(CmdList, 2);
#endif
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndiect(1, System->DispatchCommandBuffer, sizeof(DispatchArgs), nullptr, 0);
#else
	CmdList->Dispatch(System->MaxParticleCount, 1, 1);
#endif
	Sync(System);
	if (System->SortShader != nullptr)
	{
		//todo:

	}
	System->RenderCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);

}

void ParticleSystemManager::StartSimulate()
{
	CmdList->ResetList();
#if PARTICLE_STATS
	CmdList->StartTimer(EGPUTIMERS::ParticleSimulation);
#endif
}

void ParticleSystemManager::StartRender()
{
	RenderList->ResetList();
#if PARTICLE_STATS
	RenderList->StartTimer(EGPUTIMERS::ParticleDraw);
#endif
}

void ParticleSystemManager::SubmitCompute()
{
#if PARTICLE_STATS
	CmdList->EndTimer(EGPUTIMERS::ParticleSimulation);
#endif
	CmdList->Execute();
	CmdList->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void ParticleSystemManager::SubmitRender(FrameBuffer* buffer)
{
#if PARTICLE_STATS
	RenderList->EndTimer(EGPUTIMERS::ParticleDraw);
#endif
	//buffer->MakeReadyForComputeUse(RenderList);
	buffer->MakeReadyForPixel(RenderList);
	RenderList->Execute();
	CmdList->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
}

void ParticleSystemManager::RenderSystem(ParticleSystem* system, FrameBuffer * BufferTarget, EEye::Type Eye /*= Eeye::Left*/)
{
	if (!system->ShouldRender)
	{
		return;
	}
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = system->RenderShader;
	desc.FrameBufferTarget = BufferTarget;
	if (DepthBuffer != nullptr)
	{
		desc.RenderTargetDesc = BufferTarget->GetPiplineRenderDesc();
		desc.RenderTargetDesc.DSVFormat = DepthBuffer->GetPiplineRenderDesc().DSVFormat;
	}
	desc.Cull = false;
	desc.Blending = true;
	desc.Mode = Full;
	RenderList->SetPipelineStateDesc(desc);
	RHIRenderPassDesc info(BufferTarget, ERenderPassLoadOp::Load);
	info.DepthSourceBuffer = DepthBuffer;
	RenderList->BeginRenderPass(info);
	RenderList->SetVertexBuffer(VertexBuffer);
	RenderList->SetConstantBufferView(ParticleRenderConstants, Eye, 2);
	system->GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::Read);
	system->GPU_ParticleData->BindBufferReadOnly(RenderList, 1);
	system->RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::IndirectArgs);
	RenderList->SetTexture(system->ParticleTexture.Get(), 3);
#if USE_INDIRECTRENDER
	RenderList->ExecuteIndiect(system->MaxParticleCount, system->RenderCommandBuffer, 0, system->CounterBuffer, 0);
#else
	for (int i = 0; i < system->MaxParticleCount; i++)
	{
		RenderList->SetRootConstant(0, 1, &i, 0);
		RenderList->DrawPrimitive(6, 1, 0, 0);
	}
#endif
	system->GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::UnorderedAccess);
	system->RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::UnorderedAccess);
	system->SwapBuffers();
	RenderList->EndRenderPass();
}


void ParticleSystemManager::Simulate()
{
	if (!RHI::GetRenderSettings()->EnableGPUParticles)
	{
		return;
	}
	StartSimulate();
	for (int i = 0; i < ParticleSystems.size(); i++)
	{
		SimulateSystem(ParticleSystems[i]);
	}
	SubmitCompute();
}


void ParticleSystemManager::Render(FrameBuffer* TargetBuffer, FrameBuffer * DepthTexture /*= nullptr*/, EEye::Type Eye/* = EEye::Left*/)
{
	if (!RHI::GetRenderSettings()->EnableGPUParticles)
	{
		return;
	}

	DepthBuffer = DepthTexture;

	StartRender();
	for (int i = 0; i < ParticleSystems.size(); i++)
	{
		RenderSystem(ParticleSystems[i], TargetBuffer, Eye);
	}
	SubmitRender(TargetBuffer);
}

void ParticleSystemManager::AddSystem(ParticleSystem * system)
{
	ParticleSystems.push_back(system);
}

void ParticleSystemManager::RemoveSystem(ParticleSystem * system)
{
	VectorUtils::Remove(ParticleSystems, system);
	//#TODO: enqueue for release!
	//#particles Handle System destruction mid frame.
}

