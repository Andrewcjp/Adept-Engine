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
	void Render() override;

	void SSAOPass();
	void Init()override;
	DeferredRenderer(int width, int height) :RenderEngine(width, height) {}
	~DeferredRenderer();

	// Inherited via RenderEngine
	virtual Camera * GetMainCam() override;
	virtual void AddGo(GameObject * g) override;
	virtual void AddLight(Light * l) override;
	void PrepareData();
	void GeometryPass();
	void RenderSkybox(bool ismain);
	void LightingPass();
	void ShadowPass();

private:
	ShaderOutput* outshader;
	FrameBuffer* FilterBuffer;

	Camera* MainCamera;
	Shader_WDeferred* DeferredWriteShader;
	Shader_Deferred* DeferredShader;
	std::vector<GameObject*> Objects;
	std::vector<Light*> Lights;
	ShadowRenderer* shadower;
	FrameBuffer* GFrameBuffer = nullptr;
	RHICommandList* WriteList = nullptr;
	Shader_Main* MainShader = nullptr;
	RHICommandList* LightingList = nullptr;
	bool once = true;
	class	PostProcessing* Post = nullptr;
	FrameBuffer*	OutputBuffer = nullptr;
	Shader_SSAO*	SSAOShader;
	GameObject* skybox;
	int SkyboxTexture;
	ShaderProgramBase* SkyboxShader;
	// Inherited via RenderEngine
	virtual void Resize(int width, int height) override;

	// Inherited via RenderEngine
	virtual Shader * GetMainShader() override;


	void BindAsRenderTarget();



	// Inherited via RenderEngine
	virtual void DestoryRenderWindow() override;

	// Inherited via RenderEngine
	virtual void FinaliseRender() override;
};

