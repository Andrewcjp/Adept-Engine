#pragma once

#include "RHI/RHI_inc_fwd.h"

class Shader_ShadowSample;
class DeviceContext;
class CullingManager;
class ShadowAtlasStorageNode;
class ShadowAtlas;


//struct PreSampleController
//{
//	std::vector<ShadowLightInteraction*> TargetInteractions;
//	FrameBuffer* PreSampleBuffer;
//	void AddLight(ShadowLightInteraction* light);
//};

class ShadowRenderer
{
	static ShadowRenderer* Instance;
public:
	ShadowRenderer();
	~ShadowRenderer();

	static ETextureFormat GetDepthType();
	static ETextureFormat GetDepthReadType();
	static void RenderPointShadows(RHICommandList * list);
	static void RenderDirectionalShadows(RHICommandList * list);

	static RHIPipeRenderTargetDesc GetCubeMapDesc();
	static RHIFrameBufferDesc GetCubeMapFBDesc(int size = 10);

	static ETextureFormat GetPreSampledTextureFormat(int Shadownumber);
	static void InvalidateAllBakedShadows();


	static void UpdateShadowID(Light * L, int D);
	static ShadowRenderer* Get();
	void UpdateShadowAsignments();
	std::vector<ShadowAtlas*> AtlasSets;
private:
	static void RenderShadowMap_GPU(Light* Lightptr, RHICommandList * list, int IndexOnGPU);
	static void RenderShadowMap_CPU(Light* LightPtr, RHICommandList * list, int IndexOnGPU);
	static void SetPointRS(RHICommandList * list, FrameBuffer* buffer);
	//called to allocate all lights to an atlas, also handles multi-gpu assignment
	void AssignAtlasData(ShadowAtlas* Node);

	
};

