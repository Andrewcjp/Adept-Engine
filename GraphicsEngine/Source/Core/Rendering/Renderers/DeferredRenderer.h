#pragma once
#include "RenderEngine.h"
#include "Rendering/Shaders/Shader_WDeferred.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/Core/FrameBuffer.h"
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

	virtual FrameBuffer* GetGBuffer() override;

private:
	void GeometryPass(RHICommandList* List, FrameBuffer* gbuffer, int eyeindex = 0);
	void LightingPass(RHICommandList* List, FrameBuffer* GBuffer, FrameBuffer* output);
	void SSAOPass();
#if ENABLE_RENDERER_DEBUGGING
	void DebugPass();
#endif
	void RenderSkybox(RHICommandList* list, FrameBuffer* Output, FrameBuffer* DepthSource);
	Shader_WDeferred* DeferredWriteShader = nullptr;
	RHICommandList* DebugList = nullptr;

	bool once = true;
};

