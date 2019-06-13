#pragma once
class LightCullingEngine
{
public:
	LightCullingEngine();
	~LightCullingEngine();
	void Init();
	void LaunchCullingForScene(EEye::Type Eye);
	static glm::ivec2 GetLightGridDim();
	void WaitForCulling(RHICommandList* list);
	void BindLightBuffer(RHICommandList* list);
	void Unbind(RHICommandList * list);
	RHIBuffer* LightBuffer = nullptr;
	//creates list of lights that might be used
	void RunLightBroadphase();
private:
	void CreateLightDataBuffer();
	//one data buffer reused 
	RHICommandList* CullingList[EEye::Limit] = { nullptr,nullptr };

	
};

