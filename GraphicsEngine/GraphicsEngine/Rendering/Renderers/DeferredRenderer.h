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
class OGLShaderProgram;
class DeferredRenderer :public RenderEngine
{
public:
	void Render() override;

	void SSAOPass();
	void Init()override;
	void InitOGL()override;
	DeferredRenderer(int width, int height) :RenderEngine(width, height) {}
	~DeferredRenderer();

	// Inherited via RenderEngine
	virtual Camera * GetMainCam() override;
	virtual void AddGo(GameObject * g) override;
	virtual void AddPhysObj(GameObject * go) override;
	virtual void AddLight(Light * l) override;
	virtual void FixedUpdatePhysx(float dtime) override;
	void GeometryPass();
	void RenderSkybox(bool ismain);
	void LightingPass();
	void ShadowPass();
	void RenderFitlerBufferOutput();
private:
	ShaderOutput* outshader;
	FrameBuffer* FilterBuffer;
	FrameBuffer_gDeferred* DeferredFrameBuffer;
	Camera* MainCamera;
	Shader_WDeferred* DeferredWriteShader;
	Shader_Deferred* DeferredShader;
	std::vector<GameObject*> Objects;
	std::vector<Light*> Lights;
	std::vector<GameObject*>PhysicsObjects;
	ShadowRenderer* shadower;
	FrameBufferSSAO* SSAOBuffer;
	Shader_SSAO*	SSAOShader;
	GameObject* skybox;
	GLuint SkyboxTexture;
	OGLShaderProgram* SkyboxShader;
	// Inherited via RenderEngine
	virtual void Resize(int width, int height) override;

	// Inherited via RenderEngine
	virtual Shader * GetMainShader() override;

	// Inherited via RenderEngine
	virtual std::vector<GameObject*> GetObjects() override;



	// Inherited via RenderEngine
	virtual void SetReflectionCamera(Camera * c) override;

	// Inherited via RenderEngine
	virtual FrameBuffer * GetReflectionBuffer() override;
	void BindAsRenderTarget();

	// Inherited via RenderEngine
	virtual ShaderOutput * GetFilterShader() override;

	// Inherited via RenderEngine
	virtual void DestoryRenderWindow() override;

	// Inherited via RenderEngine
	virtual void FinaliseRender() override;
};

