
#include "ParticleSystemManager.h"
#include "Core/Assets/ShaderCompiler.h"
#include "Rendering/Shaders/Particle/Shader_ParticleCompute.h"
#include "Rendering/Shaders/Particle/Shader_ParticleDraw.h"
#include "RHI/DeviceContext.h"
#include "Core/Assets/AssetManager.h"
#include "GPUParticleSystem.h"
#include "Core/Utils/MemoryUtils.h"
#include "FrameBuffer.h"
#include "Core/Performance/PerfManager.h"

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
	pdesc.RenderTargetDesc.RTVFormats[0] = ETextureFormat::R32G32B32A32_FLOAT;
	pdesc.RenderTargetDesc.NumRenderTargets = 1;
	pdesc.RenderTargetDesc.DSVFormat = FORMAT_D32_FLOAT;
	pdesc.ShaderInUse = ShaderCompiler::GetShader<Shader_ParticleDraw>();
	RenderList->SetPipelineStateDesc(pdesc);

#if USE_INDIRECTRENDER
	RenderSig = RHICommandSignitureDescription();
	RenderSig.ArgumentDescs.resize(2);
	RenderSig.ArgumentDescs[0].Type = INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_CONSTANT;
	RenderSig.ArgumentDescs[0].Constant.RootParameterIndex = pdesc.ShaderInUse->GetSlotForName("Index");
	RenderSig.ArgumentDescs[0].Constant.Num32BitValuesToSet = 1;
	RenderSig.ArgumentDescs[0].Constant.DestOffsetIn32BitValues = 0;
	RenderSig.ArgumentDescs[1].Type = INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DRAW;
	RenderSig.CommandBufferStide = sizeof(IndirectArgs);
	RenderList->SetCommandSigniture(RenderSig);
#endif
}


void ParticleSystemManager::PreRenderUpdate(Camera* c)
{
	for (int i = 0; i < EEye::Limit; i++)
	{
		RenderData[i].CameraUp_worldspace = glm::vec4(0, 1, 0, 1.0f);//glm::vec4(c->GetUp(), 1.0f);
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
	CmdList->UAVBarrier(system->AliveParticleIndexs);
	CmdList->UAVBarrier(system->DeadParticleIndexs);
	CmdList->UAVBarrier(system->CounterBuffer);
	CmdList->UAVBarrier(system->DispatchCommandBuffer);
	CmdList->UAVBarrier(system->AliveParticleIndexs_PostSim);
	CmdList->UAVBarrier(system->GPU_ParticleData);
	CmdList->UAVBarrier(system->RenderCommandBuffer);
}

void ParticleSystemManager::SimulateSystem(ParticleSystem * System)
{
	if (!System->ShouldSimulate || PauseVar.GetBoolValue())
	{
		return;
	}
	SCOPE_CYCLE_COUNTER_GROUP("SimulateSystem", "Particle");
	System->DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::UnorderedAccess);
	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(ShaderCompiler::GetShader<Shader_StartSimulation>()));
	CmdList->SetUAV(System->CounterBuffer, 0);
	CmdList->SetUAV(System->DispatchCommandBuffer, 1);
	CmdList->SetRootConstant(2, 1, &System->RealEmissionCount, 0);
	CmdList->Dispatch(1, 1, 1);
	Sync(System);

	System->DispatchCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);
	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(System->EmitShader));
	CmdList->SetUAV(System->CounterBuffer, 1);
	CmdList->SetUAV(System->GPU_ParticleData, 0);
	CmdList->SetUAV(System->GetPreSimList(), 2);
	CmdList->SetUAV(System->DeadParticleIndexs, 3);
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndirect(1, System->DispatchCommandBuffer, 0, nullptr, 0);
#else
	CmdList->Dispatch(System->MaxParticleCount, 1, 1);
#endif
	Sync(System);

	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(System->SimulateShader));
	float DT = Engine::GetDeltaTime();
	CmdList->SetRootConstant(System->SimulateShader->GetSlotForName("emitData"), 1, &DT, 0);
	CmdList->SetUAV(System->GPU_ParticleData, "newPosVelo");
	CmdList->SetUAV(System->CounterBuffer, "CounterBuffer");
	CmdList->SetBuffer(System->GetPreSimList(), "AliveIndexs");
	CmdList->SetUAV(System->DeadParticleIndexs, "DeadIndexs");
	CmdList->SetUAV(System->GetPostSimList(), "PostSim_AliveIndex");
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndirect(1, System->DispatchCommandBuffer, sizeof(DispatchArgs), nullptr, 0);
#else
	CmdList->Dispatch(System->MaxParticleCount, 1, 1);
#endif
	Sync(System);
	//return;
	CmdList->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(ShaderCompiler::GetShader<Shader_EndSimulation>()));
	CmdList->SetBuffer(System->GetPostSimList(),0);
	CmdList->SetUAV(System->RenderCommandBuffer, 1);
	//CmdList->SetUAV(System->CounterBuffer, "Counter");
#if USE_INDIRECTCOMPUTE
	CmdList->ExecuteIndirect(1, System->DispatchCommandBuffer, sizeof(DispatchArgs), nullptr, 0);
#else
	CmdList->Dispatch(System->MaxParticleCount, 1, 1);
#endif
	Sync(System);
	CmdList->UAVBarrier(System->DispatchCommandBuffer);
	if (System->SortShader != nullptr)
	{
		//todo:
	}
	System->RenderCommandBuffer->SetBufferState(CmdList, EBufferResourceState::IndirectArgs);

}

void ParticleSystemManager::StartSimulate()
{
	CmdList->ResetList();

}

void ParticleSystemManager::StartRender()
{
	RenderList->ResetList();	
}

void ParticleSystemManager::SubmitCompute()
{

	CmdList->Execute();
	CmdList->GetDevice()->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::Compute);
}

void ParticleSystemManager::SubmitRender(FrameBuffer* buffer)
{

	//buffer->MakeReadyForComputeUse(RenderList);
	buffer->MakeReadyForPixel(RenderList);
	RenderList->Execute();
	CmdList->GetDevice()->InsertGPUWait(EDeviceContextQueue::Compute, EDeviceContextQueue::Graphics);
}

void ParticleSystemManager::RenderSystem(ParticleSystem* system, FrameBuffer * BufferTarget, EEye::Type Eye /*= Eeye::Left*/)
{
	if (!system->ShouldRender)
	{
		return;
	}
	SCOPE_CYCLE_COUNTER_GROUP("RenderSystem", "Particle");
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = system->RenderShader;
	desc.RenderTargetDesc = BufferTarget->GetPiplineRenderDesc();
	if (DepthBuffer != nullptr)
	{
		desc.RenderTargetDesc = BufferTarget->GetPiplineRenderDesc();
		desc.RenderTargetDesc.DSVFormat = DepthBuffer->GetPiplineRenderDesc().DSVFormat;
	}
	desc.Cull = false;
	desc.BlendState = RHIBlendState::CreateBlendDefault();
	RenderList->SetPipelineStateDesc(desc);
	RHIRenderPassDesc info(BufferTarget, ERenderPassLoadOp::Load);
	info.DepthSourceBuffer = DepthBuffer;
	RenderList->BeginRenderPass(info);
#if USE_INDIRECTRENDER
	RenderList->SetCommandSigniture(RenderSig);
#endif
	RenderList->SetVertexBuffer(VertexBuffer);
	RenderList->SetConstantBufferView(ParticleRenderConstants, Eye, "ParticleData");
	system->GPU_ParticleData->SetBufferState(RenderList, EBufferResourceState::Read);
	RenderList->SetBuffer(system->GPU_ParticleData, "newPosVelo");
	system->RenderCommandBuffer->SetBufferState(RenderList, EBufferResourceState::IndirectArgs);
	RenderList->SetTexture(system->ParticleTexture.Get(), Shader_ParticleDraw::Texture);
#if USE_INDIRECTRENDER
	RenderList->ExecuteIndirect(system->MaxParticleCount, system->RenderCommandBuffer, 0, system->CounterBuffer, 0);
#else
	const int RootConstSlot = system->RenderShader->GetSlotForName("Index");
	for (int i = 0; i < system->MaxParticleCount; i++)
	{
		RenderList->SetRootConstant(RootConstSlot, 1, &i, 0);
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


	for (int i = 0; i < ParticleSystems.size(); i++)
	{
		RenderSystem(ParticleSystems[i], TargetBuffer, Eye);
	}

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

