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
	Shader_Convolution* Conv = nullptr;
	Shader_EnvMap* EnvMap = nullptr;
	std::vector<ReflectionProbe*> Probes;
	RHICommandList* StaticGenList = nullptr;
	FrameBuffer* SkyBoxBuffer = nullptr;
};

