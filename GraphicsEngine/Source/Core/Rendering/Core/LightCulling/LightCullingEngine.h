#pragma once

class CullingManager;
class LightCullingEngine
{
public:
	LightCullingEngine();
	~LightCullingEngine();
	void Init(CullingManager* m);
	void LaunchCullingForScene(EEye::Type Eye);
	static glm::ivec2 GetLightGridDim();
	void WaitForCulling(RHICommandList* list);
	void BindLightBuffer(RHICommandList* list);
	void Unbind(RHICommandList * list);
	RHIBuffer* LightCullingBuffer = nullptr;
	//creates list of lights that might be used
	void RunLightBroadphase();
	//generates a grid of light interactions for DXR and light mapping
	void BuildLightWorldGrid();
	//Updates the Buffer which contains lights needed for this frame
	//Might cause an expand
	void UpdateLightsBuffer();
	void Resize();
	int GetNumLights() const;
private:
	RHIBuffer* LightDataBuffer = nullptr;

	void CreateLightDataBuffer();
	//one data buffer reused 
	RHICommandList* CullingList[EEye::Limit] = { nullptr,nullptr };
	std::vector<Light*> LightsInFustrum;
	CullingManager* Manager = nullptr;
	std::vector<LightUniformBuffer> LightData;
};

