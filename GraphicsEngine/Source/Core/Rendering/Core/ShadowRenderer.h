#pragma once

#include "RHI/RHI_inc_fwd.h"

class Shader_ShadowSample;
class DeviceContext;
//holds all objects need to run shadows on a device.
struct DeviceShadowObjects
{
	RHICommandList* PointLightShadowList = nullptr;
	RHITextureArray* ShadowCubeArray = nullptr;
	RHITextureArray* ShadowDirectionalArray = nullptr;
	RHIBuffer* GeometryProjections = nullptr;
	RHICommandList*  DirectionalShadowList = nullptr;
	RHICommandList* ShadowPreSamplingList = nullptr;
	void Release();
};
class ShadowRenderer
{
public:
	ShadowRenderer(class SceneRenderer* SceneRender);
	~ShadowRenderer();
	void UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index, int DeviceIndex);
	void RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects, class Shader_Main* mainshader = nullptr);
	void RenderOnDevice(DeviceContext * con, const std::vector<GameObject*>& ShadowObjects);
	void RunPointShadowPass(RHICommandList * List, const std::vector<GameObject*>& ShadowObjects);
	void PreSampleShadows(RHICommandList* list, const std::vector<GameObject*>& ShadowObjects);
	void RenderPointShadows(RHICommandList * list, const std::vector<GameObject*>& ShadowObjects);
	void RenderDirectionalShadows(RHICommandList * list, const std::vector<GameObject *> & ShadowObjects);
	void BindShadowMapsToTextures(RHICommandList* list);
	void ClearShadowLights();
	void InitShadows(std::vector<Light*> lights);
	bool UseCache = false;
	bool Renderered = false;
	void Unbind(RHICommandList* list);
	void SetupOnDevice(DeviceContext* Context);
private:
	std::vector<Light*> ShadowingDirectionalLights;
	std::vector<Light*> ShadowingPointLights;

	Shader_Depth* PointLightShader = nullptr;
	Shader_Depth* DirectionalLightShader = nullptr;

	FrameBuffer* DirectionalLightBuffer = nullptr;
	std::vector<FrameBuffer*> DirectionalLightBuffers;
	struct ShadowLightInteraction
	{
		ShadowLightInteraction(DeviceContext * Context, bool IsPoint, int MapSize);
		~ShadowLightInteraction();
		void SetupCopy(DeviceContext* TargetDev);
		Shader_Depth* Shader = nullptr;
		Light* lightPtr = nullptr;

		FrameBuffer* ShadowMap = nullptr;
		bool IsPointLight = false;
		int DeviceIndex = 0;
		DeviceContext* DevContext;
		bool SampleOnAllDevices = false;
		//PreSampled Buffer used to reduce Data transfer	
		FrameBuffer* PreSampledBuffer = nullptr;
		bool NeedsSample = false;
		int TargetDeviceIndex = 0;

		bool IsResident(DeviceContext* dev)const;
	};
	std::vector<ShadowLightInteraction*> LightInteractions;
	Shader_ShadowSample* ShadowPreSampleShader = nullptr;
	SceneRenderer* Scenerenderer = nullptr;

	DeviceShadowObjects DSOs[MAX_GPU_DEVICE_COUNT];
	bool AllDevicesNeedToRead = false;
};

