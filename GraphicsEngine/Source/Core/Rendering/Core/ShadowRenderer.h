#pragma once

#include "RHI/RHI_inc_fwd.h"

class Shader_ShadowSample;
class DeviceContext;
class CullingManager;

//holds all objects need to run shadows on a device.
struct DeviceShadowObjects
{
	RHICommandList* PointLightShadowList = nullptr;
	RHITextureArray* ShadowCubeArray = nullptr;
	RHITextureArray* ShadowDirectionalArray = nullptr;
	RHIBuffer* GeometryProjections = nullptr;
	RHICommandList*  DirectionalShadowList = nullptr;
	RHICommandList* ShadowPreSamplingList = nullptr;
	FrameBuffer* PreSampledBuffer = nullptr;
	void Release();
};
struct ShadowLightInteraction
{
	ShadowLightInteraction(Light* light);
	void CreateRenderTargets(DeviceContext * context);
	~ShadowLightInteraction();
	void SetupCopy(DeviceContext* TargetDev);
	Shader_Depth* Shader = nullptr;
	Light* lightPtr = nullptr;
	//holds All static items render by this light

	bool IsPointLight = false;
	int DeviceIndex = 0;
	DeviceContext* ResidentDevContext = nullptr;
	bool SampleOnAllDevices = false;
	//PreSampled Buffer used to reduce Data transfer	

	bool NeedsSample = false;
	int TargetDeviceIndex = 0;
	FrameBuffer* GetMap(int index);
	//writes the depth from the static objects map to the dynamic map
	void CopyDepth();
	bool IsResident(DeviceContext* dev)const;
	bool NeedsRenderOnDevice(DeviceContext* dev);
	bool NeedsPresample(DeviceContext* dev);
	bool NeedsUpdate();
	void Invalidate();
	bool captured = false;
private:
	FrameBuffer* StaticShadowMap[MAX_GPU_DEVICE_COUNT] = { nullptr };
	FrameBuffer* ShadowMap[MAX_GPU_DEVICE_COUNT] = { nullptr };
};
struct PreSampleController
{
	std::vector<ShadowLightInteraction*> TargetInteractions;
	FrameBuffer* PreSampleBuffer;
	void AddLight(ShadowLightInteraction* light);
};
#define USE_GS_FOR_CUBE_SHADOWS 1
class ShadowRenderer
{
	static ShadowRenderer* Instance;
public:
	ShadowRenderer(class SceneRenderer* SceneRender, CullingManager* culling);
	~ShadowRenderer();
	void UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index, int DeviceIndex);
	static eTEXTURE_FORMAT GetDepthType();
	static eTEXTURE_FORMAT GetDepthReadType();
	void RenderShadowMaps();
	bool NeedsAnyShadowUpdate();
	void RenderOnDevice(DeviceContext * con); 
	void RunPointShadowPass(RHICommandList * List);
	void AsyncCopy(int Index);
	void PreSampleShadows(RHICommandList* list);
	void RenderPointShadows(RHICommandList * list);
	void RenderDirectionalShadows(RHICommandList * list);
	void BindShadowMapsToTextures(RHICommandList* list, bool cubemap = false);

	void BindPointmaps(RHICommandList * list, int slot);

	void ClearShadowLights();
	static RHIPipeRenderTargetDesc GetCubeMapDesc();
	static RHIFrameBufferDesc GetCubeMapFBDesc(int size = 10);
	void InitShadows(std::vector<Light*> lights);
	bool UseCache = false;
	bool Renderered = false;
	void Unbind(RHICommandList* list);
	void InitPreSampled(DeviceContext * dev, DeviceContext * Targetdev);
	static eTEXTURE_FORMAT GetPreSampledTextureFormat(int deviceindex);
	void InvalidateAllBakedShadows();
	void SetupOnDevice(DeviceContext* Context);
	static PreSampleController* GetSampleController(int index);
private:
	int ViewInstancesPerDraw = 2;
	bool DeviceZeroNeedsPreSample = false;
	std::vector<Light*> ShadowingDirectionalLights;
	std::vector<Light*> ShadowingPointLights;
	CullingManager* Culling = nullptr;
	Shader_Depth* PointLightShader = nullptr;
	Shader_Depth* DirectionalLightShader = nullptr;

	FrameBuffer* DirectionalLightBuffer = nullptr;
	std::vector<FrameBuffer*> DirectionalLightBuffers;
	PreSampleController SampleControllers[MAX_GPU_DEVICE_COUNT];
	void RenderShadowMap_GPU(ShadowLightInteraction* interaction, RHICommandList * list, int IndexOnGPU);
	void RenderShadowMap_CPU(ShadowLightInteraction* interaction, RHICommandList * list, int IndexOnGPU);
	std::vector<ShadowLightInteraction*> LightInteractions;
	Shader_ShadowSample* ShadowPreSampleShader = nullptr;
	Shader_ShadowSample* ShadowPreSampleShader_GPU0 = nullptr;
	SceneRenderer* Scenerenderer = nullptr;

	DeviceShadowObjects DSOs[MAX_GPU_DEVICE_COUNT];
	bool NeedsCopyPreSample[MAX_GPU_DEVICE_COUNT] = { false,false };
	bool AllDevicesNeedToRead = false;
};

