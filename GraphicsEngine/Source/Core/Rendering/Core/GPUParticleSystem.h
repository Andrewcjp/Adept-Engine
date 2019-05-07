#pragma once
#define MAX_PARTICLES 100000
//defines all data need to simulate and render a particle system
struct ParticleSystem
{
	~ParticleSystem()
	{};
	//parameters are passed in though shaders
	Shader* EmitShader = nullptr;
	Shader* SimulateShader = nullptr;
	Shader* SortShader = nullptr;
	Shader* RenderShader = nullptr;

	//GPU data
	RHIBuffer * GPU_ParticleData = nullptr;
	RHIBuffer * EmittedParticleData = nullptr;
	RHIBuffer * DeadParticleIndexs = nullptr;
	RHIBuffer * AliveParticleIndexs = nullptr;
	RHIBuffer* AliveParticleIndexs_PostSim = nullptr;

	RHIBuffer* RenderCommandBuffer = nullptr;
	RHIBuffer* DispatchCommandBuffer = nullptr;
	//counters
	RHIBuffer * CounterBuffer = nullptr;
	SharedPtr<BaseTexture> ParticleTexture;
	void Init();
	void SetupCommandBuffer();
	//System params
	int MaxParticleCount = 1000;
	void Release();
	RHIBuffer * GetPreSimList();
	RHIBuffer * GetPostSimList();
	bool Flip = false;
	void SwapBuffers();
	float EmitCountPerSecond = 1.0f;

	bool ShouldRender = true;
	bool ShouldSimulate = true;
	void Tick(float dt);
	int RealEmissionCount = 1;
	void SetDefaultShaders();
private:

	float EmissionRate = 0.0f;
	float CurrentTimer = 0.0f;
};
