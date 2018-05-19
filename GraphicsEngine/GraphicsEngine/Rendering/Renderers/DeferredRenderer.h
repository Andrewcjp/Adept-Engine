#pragma once
#include "RenderEngine.h"
#include "OLD/FrameBuffer_gDeferred.h"
#include "Rendering/Shaders/Shader_WDeferred.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Rendering/Core/ShadowRenderer.h"
#include "OLD/FrameBufferSSAO.h"
#include "Rendering/Shaders/Shader_SSAO.h"
#include "../Rendering/Core/FrameBuffer.h"
#include "Rendering/Shaders/ShaderOutput.h"
class DeferredRenderer :public RenderEngine
{
public:
	void OnRender() override;

	
	void PostInit()override;
	DeferredRenderer(int width, int height) :RenderEngine(width, height) {}
	virtual ~DeferredRenderer();

	virtual void Resize(int width, int height) override;
	virtual void DestoryRenderWindow() override;
	virtual void FinaliseRender() override;
	virtual void OnStaticUpdate() override;


private:
	void GeometryPass();
	void RenderSkybox(bool ismain);
	void LightingPass();
	void SSAOPass();

	Shader_WDeferred* DeferredWriteShader = nullptr;
	Shader_Deferred* DeferredShader = nullptr;
	FrameBuffer* GFrameBuffer = nullptr;
	RHICommandList* WriteList = nullptr;
	RHICommandList* LightingList = nullptr;
	bool once = true;
	
	FrameBuffer*	OutputBuffer = nullptr;
	Shader_SSAO*	SSAOShader;
	

	
};

