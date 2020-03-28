#pragma once

class ReflectionProbe;
class RHICommandList;
class Shader_Convolution;
class Shader_EnvMap;
class ReflectionEnviroment
{
public:
	ReflectionEnviroment();
	~ReflectionEnviroment();
	///update all probes etc.
	void Update();
	void UpdateRelflectionProbes(RHICommandList * commandlist);
	bool AnyProbesNeedUpdate();
	void RenderCubemap(ReflectionProbe * Map, RHICommandList * commandlist);

	void DownSampleAndBlurProbes(RHICommandList * ComputeList);

	void BindDynamicReflections(RHICommandList * List, bool IsDeferredshader);

	void BindStaticSceneEnivoment(RHICommandList* List, bool IsDeferredshader);
	void GenerateStaticEnvData();
private:

	std::vector<ReflectionProbe*> Probes;
	RHICommandList* StaticGenList = nullptr;
	struct ReflectionEnviromentGPUData
	{
		void init(int index);

		FrameBuffer* SkyBoxBuffer = nullptr;
		Shader_Convolution* Conv = nullptr;
		Shader_EnvMap* EnvMap = nullptr;
	};
	ReflectionEnviromentGPUData GpuData[MAX_GPU_DEVICE_COUNT];
};

