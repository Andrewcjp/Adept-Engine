#include "Stdafx.h"
#include "GPUParticleSystem.h"
#include "Core\Assets\AssetManager.h"
#include "RHI\RHICommandList.h"
#include "Core\IRefCount.h"
#include "ParticleSystemManager.h"
#include "Core\Assets\ShaderComplier.h"
#include "..\Shaders\Particle\Shader_ParticleCompute.h"
#include "..\Shaders\Particle\Shader_ParticleDraw.h"

void ParticleSystem::SwapBuffers()
{
	Flip = !Flip;
}

void ParticleSystem::Tick(float DT)
{
	EmissionRate = 1.0f / EmitCountPerSecond;
	CurrentTimer -= DT;
	if (CurrentTimer >= 0)
	{
		RealEmissionCount = 0;
	}
	else
	{
		int Count = glm::max(1, glm::iround(DT*EmitCountPerSecond));
		CurrentTimer = EmissionRate;
		RealEmissionCount = Count;
	}
}

void ParticleSystem::SetDefaultShaders()
{
	SimulateShader = ShaderComplier::GetShader<Shader_ParticleCompute>();
	EmitShader = ShaderComplier::GetShader<Shader_ParticleEmit>();
	SortShader = nullptr;
	RenderShader = ShaderComplier::GetShader<Shader_ParticleDraw>();
}

void ParticleSystem::Init()
{
	SetDefaultShaders();
	MaxParticleCount = glm::min(MaxParticleCount, MAX_PARTICLES);
	//return;
	GPU_ParticleData = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	RHIBufferDesc desc;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.Stride = sizeof(ParticleSystemManager::ParticleData);
	desc.ElementCount = MaxParticleCount;
	desc.AllowUnorderedAccess = true;
	desc.CreateUAV = true;
	GPU_ParticleData->SetDebugName("Particle Data");
	GPU_ParticleData->CreateBuffer(desc);


	CounterBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(ParticleSystemManager::Counters);
	desc.ElementCount = 1;
	desc.CreateUAV = true;
	CounterBuffer->SetDebugName("Counter");
	CounterBuffer->CreateBuffer(desc);
	ParticleSystemManager::Counters count = ParticleSystemManager::Counters();
	count.deadCount = MaxParticleCount;
	CounterBuffer->UpdateIndexBuffer(&count, sizeof(ParticleSystemManager::Counters));

	SetupCommandBuffer();

	AliveParticleIndexs = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(unsigned int);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.ElementCount = MaxParticleCount;
	desc.CreateUAV = true;
	AliveParticleIndexs->SetDebugName("Alive Pre sim");
	AliveParticleIndexs->CreateBuffer(desc);

	AliveParticleIndexs_PostSim = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(unsigned int);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::GPUOnly;
	desc.ElementCount = MaxParticleCount;
	desc.CreateUAV = true;
	AliveParticleIndexs_PostSim->SetDebugName("Alive Post Sim");
	AliveParticleIndexs_PostSim->CreateBuffer(desc);

	DeadParticleIndexs = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc = RHIBufferDesc();
	desc.Stride = sizeof(unsigned int);
	desc.AllowUnorderedAccess = true;
	desc.Accesstype = EBufferAccessType::Static;
	desc.ElementCount = MaxParticleCount;
	desc.CreateUAV = true;
	DeadParticleIndexs->SetDebugName("Dead particle indexes");
	DeadParticleIndexs->CreateBuffer(desc);
	uint32_t* indices = new uint32_t[MAX_PARTICLES];
	for (uint32_t i = 0; i < MAX_PARTICLES; ++i)
	{
		indices[i] = i;
	}
	DeadParticleIndexs->UpdateBufferData(indices, MAX_PARTICLES, EBufferResourceState::UnorderedAccess);

	delete[] indices;

	//test
	if (ParticleTexture == nullptr)
	{
		ParticleTexture = AssetManager::DirectLoadTextureAsset("texture\\smoke.png", false);
	}
}

void ParticleSystem::SetupCommandBuffer()
{
	DispatchCommandBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	RHIBufferDesc desc;
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(IndirectDispatchArgs) * 2;
	desc.ElementCount = 1;
	desc.AllowUnorderedAccess = true;
	desc.CreateUAV = true;
	DispatchCommandBuffer->SetDebugName("Dispatch Command Buffer");
	DispatchCommandBuffer->CreateBuffer(desc);
	ParticleSystemManager::DispatchArgs Data[2];
	Data[0].dispatchArgs = IndirectDispatchArgs();
	Data[0].dispatchArgs.ThreadGroupCountX = 1;
	Data[0].dispatchArgs.ThreadGroupCountY = 1;
	Data[0].dispatchArgs.ThreadGroupCountZ = 1;
	Data[1].dispatchArgs = IndirectDispatchArgs();
	Data[1].dispatchArgs.ThreadGroupCountX = 1;
	Data[1].dispatchArgs.ThreadGroupCountY = 1;
	Data[1].dispatchArgs.ThreadGroupCountZ = 1;
	DispatchCommandBuffer->UpdateBufferData(Data, sizeof(IndirectDispatchArgs) * 2, EBufferResourceState::UnorderedAccess);



	RenderCommandBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	desc.Accesstype = EBufferAccessType::Static;
	desc.Stride = sizeof(ParticleSystemManager::IndirectArgs);
	desc.ElementCount = MaxParticleCount;
	desc.AllowUnorderedAccess = true;
	desc.CreateUAV = true;
	RenderCommandBuffer->SetDebugName("RenderCommandBuffer");
	RenderCommandBuffer->CreateBuffer(desc);
	std::vector<ParticleSystemManager::IndirectArgs> cmdbuffer;
	cmdbuffer.resize(MaxParticleCount);
	for (int i = 0; i < cmdbuffer.size(); i++)
	{
		cmdbuffer[i].drawArguments.VertexCountPerInstance = 6;
		cmdbuffer[i].drawArguments.InstanceCount = 1;
		cmdbuffer[i].drawArguments.StartVertexLocation = 0;
		cmdbuffer[i].drawArguments.StartInstanceLocation = 0;
		cmdbuffer[i].data = 0;
	}
	RenderCommandBuffer->UpdateBufferData(cmdbuffer.data(), cmdbuffer.size(), EBufferResourceState::UnorderedAccess);
}

void ParticleSystem::Release()
{
	EnqueueSafeRHIRelease(GPU_ParticleData);
	EnqueueSafeRHIRelease(RenderCommandBuffer);
	EnqueueSafeRHIRelease(CounterBuffer);
	EnqueueSafeRHIRelease(DispatchCommandBuffer);
	EnqueueSafeRHIRelease(AliveParticleIndexs);
	EnqueueSafeRHIRelease(DeadParticleIndexs);
	EnqueueSafeRHIRelease(AliveParticleIndexs_PostSim);
}

RHIBuffer* ParticleSystem::GetPreSimList()
{
	if (Flip)
	{
		return AliveParticleIndexs_PostSim;
	}
	return AliveParticleIndexs;
}

RHIBuffer* ParticleSystem::GetPostSimList()
{
	if (Flip)
	{
		return AliveParticleIndexs;
	}
	return AliveParticleIndexs_PostSim;
}