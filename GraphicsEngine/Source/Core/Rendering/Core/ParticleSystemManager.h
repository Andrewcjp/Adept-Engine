#pragma once
#include "RHI/RHI.h"
#define MAX_PARTICLES 1000000
#define USE_INDIRECTCOMPUTE 1
#define USE_INDIRECTRENDER 1
#define PARTICLE_STATS 1
class ParticleSystemManager
{
public:
	ParticleSystemManager();
	~ParticleSystemManager();
	void Init();
	void SetupCommandBuffer();
	void PreRenderUpdate(Camera * c);
	struct ParticleData
	{
		glm::vec4 Pos;
		glm::vec4 Vel;
		float lifetime;
		float Size;
	};
	struct IndirectArgs
	{
		int data;
		IndirectDrawArgs drawArguments;
	};
	struct DispatchArgs
	{
		IndirectDispatchArgs dispatchArgs;
	};
	static ParticleSystemManager* Get();
	void ShutDown();
	void Sync();
	RHIBuffer * GetPreSimList();
	RHIBuffer * GetPostSimList();
	void Simulate();
	void Render(FrameBuffer * BufferTarget);
private:
	RHIGPUSyncEvent* ComputeCompleteEvent = nullptr;
	static ParticleSystemManager* Instance;
	RHIBuffer * GPU_ParticleData = nullptr;
	RHIBuffer * EmittedParticleData = nullptr;
	RHIBuffer * DeadParticleIndexs = nullptr;
	RHIBuffer * AliveParticleIndexs = nullptr;
	RHIBuffer* AliveParticleIndexs_PostSim = nullptr;
	int CurrentParticleCount = MAX_PARTICLES;
	RHICommandList* CmdList = nullptr;
	RHIBuffer* RenderCommandBuffer = nullptr;
	RHIBuffer* DispatchCommandBuffer = nullptr;
	//counters
	RHIBuffer * CounterBuffer = nullptr;
	struct Counters
	{
		unsigned int aliveCount = 0;
		unsigned int deadCount = 0;
		unsigned int realEmitCount = 0;
		unsigned int aliveCount_afterSimulation = 0;
	};

	//rendering
	RHICommandList* RenderList = nullptr;
	RHIBuffer* VertexBuffer = nullptr;
	RHIBuffer* ParticleRenderConstants = nullptr;
	struct ParticleConstData
	{
		glm::mat4x4 VPMat;
		glm::vec4 CameraRight_worldspace;
		glm::vec4 CameraUp_worldspace;
	};
	ParticleConstData RenderData = ParticleConstData();
	int emitcount = 0;
	//TEst
	BaseTexture* TEstTex = nullptr;
	bool Flip = false;
};

