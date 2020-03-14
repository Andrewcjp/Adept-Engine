#pragma once
#include "../SceneRenderer.h"

class CullingManager;
class RHIBufferGroup;
//one of the few classes that splits On GPUs
class LightCullingEngine
{
public:
	LightCullingEngine();
	~LightCullingEngine();
	void Init(CullingManager* m);
	void LaunchCullingForScene(RHICommandList* list, EEye::Type Eye);
	static glm::ivec2 GetLightGridDim();
	void WaitForCulling(RHICommandList* list);
	void BindLightBuffer(RHICommandList* list, bool deferred = false);
	RHIBufferGroup * GetLightDataBuffer();
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
	std::vector<Light*> GetCurrentlyRelevantLights();
private:
	struct GPUData
	{
		RHIBuffer* LightDataBuffer = nullptr;
		RHIBuffer* CulledIndexLightBuffer = nullptr;
	};
	RHIBufferGroup* LightDataBuffer = nullptr;

	void CreateLightDataBuffer();
	//one data buffer reused 
	RHICommandList* CullingList[EEye::Limit] = { nullptr,nullptr };
	std::vector<Light*> LightsInFustrum;
	CullingManager* Manager = nullptr;
	std::vector<LightUniformBuffer> LightData;
};

