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
	ShadowRenderer(class SceneRenderer* SceneRender, CullingManager* culling);
	~ShadowRenderer();

	static eTEXTURE_FORMAT GetDepthType();
	static eTEXTURE_FORMAT GetDepthReadType();
	void PreSampleShadows(RHICommandList* list);
	static void RenderPointShadows(RHICommandList * list);
	void RenderDirectionalShadows(RHICommandList * list);

	static RHIPipeRenderTargetDesc GetCubeMapDesc();
	static RHIFrameBufferDesc GetCubeMapFBDesc(int size = 10);

	static eTEXTURE_FORMAT GetPreSampledTextureFormat(int Shadownumber);
	static void InvalidateAllBakedShadows();

	//called to allocate all lights to an atlas, also handles multi-gpu assignment
	static void AssignAtlasData(ShadowAtlas* Node);

private:
	static void RenderShadowMap_GPU(Light* Lightptr, RHICommandList * list, int IndexOnGPU);
	static void RenderShadowMap_CPU(Light* LightPtr, RHICommandList * list, int IndexOnGPU);
	static void SetPointRS(RHICommandList * list);

};

