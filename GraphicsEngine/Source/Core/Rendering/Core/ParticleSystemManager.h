#pragma once
#include "RHI/RHI.h"
#define MAX_PARTICLES 100
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
	};
	struct IndirectArgs
	{
		int data;
		D3D12_DRAW_ARGUMENTS drawArguments;
	};
	struct IndrectDispatchArgs
	{
		D3D12_DISPATCH_ARGUMENTS dispatchArgs;
	};
	static ParticleSystemManager* Get();
	void ShutDown();
	void Simulate();
	void Render(FrameBuffer * BufferTarget);
private:
	static ParticleSystemManager* Instance;
	RHIBuffer * GPU_ParticleData = nullptr;
	RHIBuffer * EmittedParticleData = nullptr;
	RHIBuffer * DeadParticleIndexs = nullptr;
	RHIUAV * DeadParticleIndexs_UAV = nullptr;
	RHIBuffer * AliveParticleIndexs = nullptr;
	RHIUAV * AliveParticleIndexs_UAV = nullptr;

	int CurrentParticleCount = 100;
	RHIUAV* GPU_ParticleData_UAV = nullptr;
	RHICommandList* CmdList = nullptr;
	RHIBuffer* RenderCommandBuffer = nullptr;
	RHIUAV* RenderCommandBuffer_UAV = nullptr;
	RHIBuffer* DispatchCommandBuffer = nullptr;
	RHIUAV* DispatchCommandBuffer_UAV = nullptr;
	//counters
	RHIBuffer * CounterBuffer = nullptr;
	RHIUAV* CounterUAV = nullptr;
	struct Counters
	{
		UINT aliveCount = 0;
		UINT deadCount = 0;
		UINT realEmitCount = 0;
		UINT aliveCount_afterSimulation = 0;
	};

	//rendering
	RHICommandList* RenderList = nullptr;
	RHIBuffer* VertexBuffer = nullptr;
	RHIBuffer* ParticleRenderConstants = nullptr;
	struct ParticleConstData
	{
		glm::mat4x4 VPMat;
		glm::vec3 CameraRight_worldspace;
		glm::vec3 CameraUp_worldspace;
	};
	ParticleConstData RenderData = ParticleConstData();
	int emitcount = 0;
};

