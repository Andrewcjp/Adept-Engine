#pragma once
#include "RenderEngine.h"


#include "OpenGL/OGLShader.h"

#include <vector>
#include "../Rendering/Core/Camera.h"
#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "../Rendering/Core/Light.h"

#include "../D3D12/D3D12RHI.h"
#include "../Rendering/Core/Material.h"
#include "RHI/Shader.h"
#include "Rendering\Shaders\Shader_Main.h"
#include "Rendering\Shaders\Shader_Depth.h"
#include "Rendering\Shaders\Shader_Grass.h"
#include "Rendering\Shaders\Shader_Querry.h"
#include "Core/Assets/Scene.h"
#include "Rendering\Shaders\Shader_Water.h"
#include "OpenGL/OGLPlane.h"
#include "Rendering\Shaders\ShaderOutput.h"
#include "../Rendering/Core/FrameBuffer.h"
#include "../Rendering/Core/ParticleSystem.h"
#include <memory>
#include "../Rendering/Core/ShadowRenderer.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Rendering/Core/GrassPatch.h"
#include "Core/Assets/ImageLoader.h"
#include "Rendering\Shaders\Shader_Skybox.h"
#include "OpenGL/OGLFrameBuffer.h"
#include "../Core/Performance/PerfManager.h"
#include "../EngineGlobals.h"
class OGLShaderProgram;
class ForwardRenderer : public RenderEngine
{
public:
	ForwardRenderer(int width, int height);
	void RunQuery();
	void Resize(int width, int height);

	~ForwardRenderer();
	void Render() override;
	std::vector<GameObject*> InGetObj();
	void UpdateDeltaTime(float value)
	{
		deltatime = value;
	}
	void Init()override;
	void ReflectionPass();
	void ShadowPass();
	void BindAsRenderTarget();
	void PrepareData();
	void RenderDebugPlane();
	void MainPass();
	void RenderSkybox(bool ismain = true);
	void RenderFitlerBufferOutput();
	void InitOGL()override;
	virtual std::vector<GameObject*> GetObjects() override
	{
		return *mainscene->GetObjects();
	}
	void SetScene(Scene * sc) override;

	float deltatime = 1;
	clock_t tstart;

	Camera* GetMainCam() override
	{
		return MainCamera;
	}
	void AddGo(GameObject* g)override
	{
		mainscene->AddGameobjectToScene(g);
//1		ob.push_back(g);
	}
	void AddPhysObj(GameObject* go) override
	{
		if (PhysicsObjects.size() >= MaxPhysicsObjects)
		{
			PhysicsObjects.erase(PhysicsObjects.begin());
		}
		PhysicsObjects.push_back(go);

	}
	void AddLight(Light* l) override
	{
		mainscene->GetLights()->push_back(l);
		//	Lights.push_back(l);
	}
	void FixedUpdatePhysx(float dtime) override
	{
		deltatime = dtime;
		//todo: Move to Compoenent;
		//todo: Update Objects
		if (RenderGrass)
		{
			grasstest->UpdateAnimation(deltatime);
		}
		if (LoadParticles && RenderParticles)
		{
			particlesys->Add(deltatime);
			particlesys->Simulate(deltatime, MainCamera->GetPosition());
		}
	}
	Camera* RefelctionCamera;
	FrameBuffer* RelfectionBuffer;
	ShaderOutput* outshader;
	bool RenderParticles = true;
	bool RenderGrass = true;
	bool UseQuerry = false;
	void SetRenderSettings(RenderSettings settings) override;
private:
#if BUILD_D3D12
	CommandListDef* MainList;
	CommandListDef* ShadowList;
#endif
	FrameBuffer* FilterBuffer;
	std::unique_ptr<ParticleSystem> particlesys;
	std::unique_ptr<GrassPatch>  grasstest;
	std::vector<GameObject*>* Objects;
	class D3D12Plane* debugplane;
	//std::vector<GameObject*> ob;
	std::vector<GameObject*> PhysicsObjects;
	//TextRenderer
	Camera*     MainCamera;
	std::vector<Light*>* Lights;
	float FrameBufferRatio = 1;
	
	GameObject* skybox;
	GameObject* playerGO;
	ShadowRenderer* shadowrender;
	D3D12RHI * DRHI;
	Shader_Main* mainshader;
	Shader_Grass* grassshader;
	Shader_Water* Watershader;
	Shader* QuerryShader;
	GameObject* testGrassobject;
	std::unique_ptr<Shader_Skybox> skyboxShader;
	bool RenderedReflection = false;
	bool LoadGrass = true;

	bool LoadParticles = true;


	int ReflectionBufferWidth = 512;
	int ReflectionBufferHeight = 512;
	// Inherited via RenderEngine
	virtual Shader_Main * GetMainShader() override;

	// Inherited via RenderEngine
	virtual void SetReflectionCamera(Camera * c) override;


	// Inherited via RenderEngine
	virtual FrameBuffer * GetReflectionBuffer() override;


	// Inherited via RenderEngine
	virtual ShaderOutput * GetFilterShader() override;
	bool once = true;

	// Inherited via RenderEngine
	virtual void DestoryRenderWindow() override;



	// Inherited via RenderEngine
	virtual void FinaliseRender() override;

};

