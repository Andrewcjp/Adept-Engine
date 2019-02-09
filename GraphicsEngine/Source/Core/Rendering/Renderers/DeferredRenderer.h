#pragma once
#include "RenderEngine.h"
#include "Rendering/Shaders/Shader_WDeferred.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/Shaders/Shader_SSAO.h"
#include "Rendering/Core/FrameBuffer.h"
struct DeferredDeviceObjects
{
	FrameBuffer* GFrameBuffer = nullptr;
	RHICommandList* WriteList = nullptr;
	RHICommandList* LightingList = nullptr;
	FrameBuffer*	OutputBuffer = nullptr;
	Shader_Deferred* DeferredShader = nullptr;
	void Release();
};

class DeferredRenderer :public RenderEngine
{
public:
	void OnRender() override;
	void RenderOnDevice(DeviceContext * con);
	void PostInit()override;
	void SetUpOnDevice(DeviceContext * con);
	DeferredRenderer(int width, int height) :RenderEngine(width, height)
	{}
	virtual ~DeferredRenderer();
	virtual void Resize(int width, int height) override;
	virtual void DestoryRenderWindow() override;
	virtual void FinaliseRender() override;
	virtual void OnStaticUpdate() override;
private:
	void GeometryPass(RHICommandList* List);
	void LightingPass(RHICommandList* List);
	void SSAOPass();
	void RenderSkybox(DeviceContext* con);
	Shader_WDeferred* DeferredWriteShader = nullptr;
	

	bool once = true;

	Shader_SSAO*	SSAOShader;
	DeferredDeviceObjects DDDOs[MAX_GPU_DEVICE_COUNT];
};

