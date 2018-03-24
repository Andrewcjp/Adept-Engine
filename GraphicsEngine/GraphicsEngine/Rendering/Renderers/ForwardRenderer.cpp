#include "ForwardRenderer.h"
#include "OpenGL/OGLShaderProgram.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"
#include "../Editor/Editor_Camera.h"
#include "../EngineGlobals.h"

#include "../Core/Engine.h"
#define USED3D12DebugP 0
#if USED3D12DebugP
#include "../D3D12/D3D12Plane.h"
#endif
ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{
	FrameBufferRatio = 1;
	if (RHI::GetType() == RenderSystemD3D11)
	{
		LoadGrass = false;
		RenderGrass = false;
		LoadParticles = false;
		RenderParticles = false;
	}
	else
	{
		LoadGrass = true;
		RenderGrass = true;
		LoadParticles = true;
		RenderParticles = false;
	}
	if (RHI::GetType() == RenderSystemD3D12)
	{
		LoadGrass = false;
		RenderGrass = false;
		LoadParticles = false;
		RenderParticles = false;
#if USED3D12DebugP
		debugplane = new D3D12Plane(1);
#endif
	}
}
void ForwardRenderer::RunQuery()
{
	glColorMask(false, false, false, false);
	glDepthMask(GL_FALSE);

	QuerryShader->SetShaderActive();
	for (size_t i = 0; i < Objects->size(); i++)
	{
		if ((InGetObj())[i]->QuerryWait)
		{
			continue;
		}

		QuerryShader->UpdateUniforms((InGetObj())[i]->GetTransform(), MainCamera, (*Lights));
		//QuerryShader->SetNormalState((Objects[i]->GetMat()->NormalMap != nullptr), (Objects[i]->GetMat()->DisplacementMap != nullptr), (Objects[i]->GetReflection() == true));//this enables 
		glBeginQuery(GL_SAMPLES_PASSED, (InGetObj())[i]->Querry);
		// Every pixel that passes the depth test now gets added to the result
		(InGetObj())[i]->Render();
		glEndQuery(GL_SAMPLES_PASSED);
		(InGetObj())[i]->QuerryWait = true;
		int iSamplesPassed = 0;
		glGetQueryObjectiv((InGetObj())[i]->Querry, GL_QUERY_RESULT, &iSamplesPassed);
		(InGetObj())[i]->Occluded = (iSamplesPassed == 0);
	}
	glColorMask(true, true, true, true);
	glDepthMask(GL_TRUE);
}
void ForwardRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (RHI::GetType() == RenderSystemOGL)
	{
		glViewport(0, 0, width, height);
	}
	if (RHI::IsD3D12())
	{
		if (DRHI)
		{
			DRHI->ResizeSwapChain(width, height);
		}
	}
	outshader->Resize(width, height);
	if (FilterBuffer != nullptr)
	{
		delete FilterBuffer;
		FilterBuffer = RHI::CreateFrameBuffer(width, height, FrameBufferRatio);
	}
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)width / (float)height);
	}
}

ForwardRenderer::~ForwardRenderer()
{}

void ForwardRenderer::Render()
{
	if (Objects == nullptr)
	{
		return;
	}
	if (Objects->size() == 0)
	{
		return;
	}
	if (MainCamera == nullptr)
	{
		return;
	}
#if WITH_EDITOR
	//todo!
	if (EditorCam != nullptr && EditorCam->GetEnabled())
	{
		if (MainCamera != EditorCam->GetCamera())
		{
			MainCamera = EditorCam->GetCamera();
		}
	}
	else
	{
		/*if (MainCamera == EditorCam->GetCamera())
		{
			MainCamera = mainscene->GetCurrentRenderCamera();
		}*/
	}
#endif
	ShadowPass();
	RHI::ClearColour();
	RHI::ClearDepth();
	RenderSkybox();
	if (RenderedReflection == false)
	{
		//ReflectionPass();
	}
	RHI::ClearColour();
	RHI::ClearDepth();

	MainPass();
	RenderSkybox();
}
std::vector<GameObject*> ForwardRenderer::InGetObj()
{
	return (*Objects);
}
void ForwardRenderer::UpdateDeltaTime(float value)
{
	deltatime = value;
}
void ForwardRenderer::Init()
{
	RHI::InitRenderState();
	mainshader = new Shader_Main();
	if (RHI::GetType() == RenderSystemOGL)
	{
		QuerryShader = new Shader_Querry();
	}
	FilterBuffer = RHI::CreateFrameBuffer(m_width, m_height, FrameBufferRatio);
	outshader = new ShaderOutput(FilterBuffer->GetWidth(), FilterBuffer->GetHeight());
	RelfectionBuffer = RHI::CreateFrameBuffer(ReflectionBufferWidth, ReflectionBufferHeight);
	if (LoadParticles)
	{
		particlesys = std::make_unique<ParticleSystem>();
	}
	if (LoadGrass)
	{
		grassshader = new Shader_Grass();
	}
	skyboxShader = std::make_unique<Shader_Skybox>();
	if (mainscene == nullptr)
	{
		mainscene = new Scene();
	}
	Lights = mainscene->GetLights();
	(Objects) = (mainscene->GetObjects());
	shadowrender = new ShadowRenderer();
	ShadowCMDList = RHI::CreateCommandList();
	shadowrender->InitShadows((*Lights), ShadowCMDList);
	GPUStateCache::Create();

	/*skybox = new GameObject();
	skybox->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("skybox.obj", GetMainShader()->GetShaderProgram(), true), nullptr));*/
	if (LoadGrass)
	{
		grasstest = std::make_unique<GrassPatch>();
	}
	TESTINIT();
}

void ForwardRenderer::ReflectionPass()
{
	if (RefelctionCamera == nullptr)
	{
		return;
	}
	bool shouldExec = false;
	for (size_t i = 0; i < (InGetObj()).size(); i++)
	{
		if ((InGetObj())[i]->GetReflection() == true && !(InGetObj())[i]->Occluded)
		{
			shouldExec = true;
			continue;
		}
	}
	if (!shouldExec)
	{
		return;
	}
	RelfectionBuffer->BindBufferAsRenderTarget();
	RHI::ClearColour();
	RHI::ClearDepth();


	mainshader->SetShaderActive();
	//	shadowrender->BindShadowMaps();

	for (size_t i = 0; i < (InGetObj()).size(); i++)
	{
		if ((InGetObj())[i]->GetReflection())
		{
			continue;
		}
		if (InGetObj()[i]->GetMat() == nullptr)
		{
			continue;
		}
		if (!(InGetObj())[i]->GetDoesUseMainShader())
		{
			grassshader->SetShaderActive();
			grassshader->UpdateUniforms((InGetObj())[i]->GetTransform(), RefelctionCamera, (*Lights));
			(InGetObj())[i]->Render();

			mainshader->SetShaderActive();
			continue;
		}
		if (InGetObj()[i]->GetMat() != nullptr)
		{
			mainshader->SetNormalState(((InGetObj())[i]->GetMat()->NormalMap != nullptr), ((InGetObj())[i]->GetMat()->DisplacementMap != nullptr), ((InGetObj())[i]->GetReflection() == true));//this enables non normal mapped surfaces not to be black
		}

		mainshader->UpdateUniforms((InGetObj())[i]->GetTransform(), RefelctionCamera, (*Lights));
		(InGetObj())[i]->Render();
	}
	RenderSkybox(false);
	RelfectionBuffer->UnBind();
}

void ForwardRenderer::ShadowPass()
{
	return;
	//if (ShadowList != nullptr)
	//{
	//	shadowrender->ResetCommandList(ShadowList);
	//	mainshader->BindLightsBuffer(ShadowList);
	//}
	////ShadowCMDList->ClearScreen();

	//shadowrender->RenderShadowMaps(MainCamera, (*Lights), (InGetObj()), ShadowCMDList, mainshader);
	//if (ShadowList != nullptr)
	//{
	//	ShadowList->Close();
	//	DRHI->ExecList(ShadowList);
	//}

}
void ForwardRenderer::BindAsRenderTarget()
{
	RHI::BindScreenRenderTarget(m_width, m_height);
}
void ForwardRenderer::PrepareData()
{
	for (size_t i = 0; i < (InGetObj()).size(); i++)
	{
		mainshader->UpdateUnformBufferEntry(mainshader->CreateUnformBufferEntry(InGetObj()[i]->GetTransform()), (int)i);
	}
}
void ForwardRenderer::RenderDebugPlane()
{
#if (_DEBUG) && USED3D12DebugP
	MainList->SetGraphicsRootSignature(((D3D12Shader*)outshader->GetShaderProgram())->m_Shader.m_rootSignature);
	MainList->SetPipelineState(((D3D12Shader*)outshader->GetShaderProgram())->m_Shader.m_pipelineState);
	debugplane->Render(MainList);
#endif

}
void ForwardRenderer::MainPass()
{

	if (RHI::GetType() == RenderSystemOGL)
	{
		if (UseQuerry)
		{
			RunQuery();
		}
	}
	if (RHI::GetType() == RenderSystemD3D12)
	{
		if (once)
		{
			/*DRHI->ExecSetUpList();*/
			D3D12RHI::Instance->ExecSetUpList();
			once = false;
		}
	}
	if (mainscene->StaticSceneNeedsUpdate)
	{
		shadowrender->InitShadows(*Lights, ShadowCMDList);
		shadowrender->Renderered = false;
	}
	PrepareData();
	TEST();
	return;

#if 0
	if (RHI::GetType() == RenderSystemD3D12)
	{
		PrepareData();
		DRHI->PreFrameSetUp(MainList, ((D3D12Shader*)mainshader->GetShaderProgram()));
		DRHI->PreFrameSwap(MainList);
		DRHI->ClearRenderTarget(MainList);
	}
	mainshader->RefreshLights();
	FilterBuffer->BindBufferAsRenderTarget();
	RHI::ClearColour();
	RHI::ClearDepth();
	RenderSkybox();
	FilterBuffer->ClearBuffer();
	mainshader->currentnumber += 0.1f* deltatime;
	mainshader->SetShaderActive();
	///	shadowrender->BindShadowMaps(MainList);
	int count = 0;
	if (RHI::GetType() == RenderSystemD3D12)
	{
		mainshader->UpdateCBV();
		mainshader->UpdateLightBuffer(*Lights);
		mainshader->BindLightsBuffer(MainList);
		mainshader->UpdateMV(MainCamera);
	}
	for (size_t i = 0; i < (InGetObj()).size(); i++)
	{
		if (InGetObj()[i]->GetMat() == nullptr)
		{
			continue;
		}
		if (!(InGetObj())[i]->GetDoesUseMainShader())
		{
			grassshader->SetShaderActive();
			grassshader->UpdateUniforms((InGetObj())[i]->GetTransform(), MainCamera, (*Lights));
			(InGetObj())[i]->Render();
			//	itemsrender++;
			mainshader->SetShaderActive();
			continue;
		}
		if (InGetObj()[i]->GetMat() != nullptr)
		{
			//	itemsrender++;
			mainshader->SetNormalState(((InGetObj())[i]->GetMat()->NormalMap != nullptr), ((InGetObj())[i]->GetMat()->DisplacementMap != nullptr), ((InGetObj())[i]->GetReflection() == true));//this enables non normal mapped surfaces not to be black
		}
		if ((InGetObj())[i]->GetReflection())
		{
			mainshader->ISWATER = true;
		}
		else
		{
			mainshader->ISWATER = false;
		}
		if (RHI::GetType() != RenderSystemD3D12)
		{
			mainshader->UpdateUniforms((InGetObj())[i]->GetTransform(), MainCamera, (*Lights));
		}
		else
		{
			//mainshader->SetActiveIndex(MainList, (int)i);
		}
		(InGetObj())[i]->Render(false, MainList);
	}

	for (size_t i = 0; i < PhysicsObjects.size(); i++)
	{
		float distance = (fabs(glm::length(PhysicsObjects[i]->GetTransform()->GetPos() - MainCamera->GetPosition())));
		if (PhysicsObjects[i]->CheckCulled(distance, glm::angle(PhysicsObjects[i]->GetTransform()->GetPos(), MainCamera->GetForward())))
		{
			continue;
		}
		//	itemsrender++;
		mainshader->SetNormalState((PhysicsObjects[i]->GetMat()->NormalMap != nullptr), (PhysicsObjects[i]->GetMat()->DisplacementMap != nullptr), (PhysicsObjects[i]->GetReflection() == true));
		//this enables non normal mapped surfaces not to be black
		mainshader->UpdateUniforms(PhysicsObjects[i]->GetTransform(), MainCamera, (*Lights));
		PhysicsObjects[i]->Render();
	}
	if (RenderGrass)
	{
		grasstest->UpdateUniforms(MainCamera, (*Lights), static_cast<float>(deltatime));
		grasstest->Render();
	}
	if (RenderParticles)
	{
		particlesys->UpdateUniforms(NULL, MainCamera, (*Lights));
		particlesys->Render();
	}
	if (RHI::GetType() == RenderSystemD3D12)
	{
		RenderDebugPlane();
		DRHI->PostFrame(MainList);
		/*MainList->Close();*/
		DRHI->ExecList(MainList, false);
	}
#endif

}

void ForwardRenderer::RenderSkybox(bool ismain)
{
	if (RHI::GetType() == RenderSystemD3D12)
	{
		return;
	}
	skyboxShader->SetShaderActive();
	skyboxShader->UpdateUniforms(nullptr, (ismain ? MainCamera : RefelctionCamera));

	skybox->Render();
	RHI::SetDepthMaskState(true);

}
void ForwardRenderer::RenderFitlerBufferOutput()
{
	BindAsRenderTarget();
	RHI::ClearColour();
	RHI::ClearDepth();
	outshader->SetShaderActive();
	outshader->UpdateUniforms(nullptr, MainCamera);
	FilterBuffer->BindToTextureUnit(0);
	outshader->RenderPlane();
}

std::vector<GameObject*> ForwardRenderer::GetObjects()
{
	return *mainscene->GetObjects();
}

void ForwardRenderer::SetScene(Scene * sc)
{
	RenderEngine::SetScene(sc);
	if (sc == nullptr)
	{
		MainCamera = nullptr;
		Lights = nullptr;
		Objects = nullptr;
		shadowrender->ClearShadowLights();
		return;
	}
	Lights = mainscene->GetLights();
	(Objects) = (mainscene->GetObjects());
	mainshader->RefreshLights();
	if (shadowrender != nullptr)
	{
		shadowrender->InitShadows(*Lights, ShadowCMDList);
	}
	if (shadowrender != nullptr)
	{
		shadowrender->Renderered = false;
	}
	if (sc == nullptr)
	{
		MainCamera = nullptr;
	}
	else
	{
		MainCamera = mainscene->GetCurrentRenderCamera();
	}
}

void ForwardRenderer::TESTINIT()
{
	MainCommandList = RHI::CreateCommandList();	 
	//finally init the pipeline!
	MainCommandList->CreatePipelineState(mainshader);
	shadowrender->InitShadows((*Lights), ShadowCMDList);
}

void ForwardRenderer::TEST()
{
	MainCommandList->ResetList();

	MVBuffer buffer;
	buffer.P = MainCamera->GetProjection();
	buffer.V = MainCamera->GetView();
	mainshader->UpdateMV(MainCamera);
	mainshader->UpdateLightBuffer(*Lights);
	
	mainshader->BindLightsBuffer(MainCommandList);
	mainshader->UpdateCBV();
	if (true)
	{
		
		ShadowCMDList->ResetList();
		mainshader->BindLightsBuffer(ShadowCMDList);

		shadowrender->RenderShadowMaps(MainCamera, (*Lights), (*Objects), ShadowCMDList, mainshader);
		ShadowCMDList->Execute();

		shadowrender->BindShadowMapsToTextures(MainCommandList);
	}
	mainshader->UpdateMV(MainCamera);
	MainCommandList->ClearScreen();
	MainCommandList->SetScreenBackBufferAsRT();
	for (size_t i = 0; i < (*Objects).size(); i++)
	{
		mainshader->SetActiveIndex(MainCommandList, i);
		(*Objects)[i]->Render(false, MainCommandList);
	}
	MainCommandList->Execute();
}

Camera * ForwardRenderer::GetMainCam()
{
	return MainCamera;
}

void ForwardRenderer::AddGo(GameObject * g)
{
	mainscene->AddGameobjectToScene(g);
}

void ForwardRenderer::AddLight(Light * l)
{
	mainscene->GetLights()->push_back(l);
}

void ForwardRenderer::FixedUpdatePhysx(float dtime)
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

void ForwardRenderer::SetRenderSettings(RenderSettings set)
{
	settings = set;
	FrameBufferRatio = set.RenderScale;
	outshader->SetFXAA((set.CurrentAAMode == AAMode::FXAA));
}

Shader_Main * ForwardRenderer::GetMainShader()
{
	return mainshader;
}

void ForwardRenderer::SetReflectionCamera(Camera * c)
{
	RefelctionCamera = c;
}

FrameBuffer * ForwardRenderer::GetReflectionBuffer()
{
	return RelfectionBuffer;
}

ShaderOutput * ForwardRenderer::GetFilterShader()
{
	return outshader;
}

void ForwardRenderer::DestoryRenderWindow()
{

	for (int i = 0; i < (InGetObj()).size(); i++)
	{
		delete (InGetObj())[i];
	}
	(InGetObj()).clear();

	delete mainshader;
	delete QuerryShader;
	delete grassshader;
	delete FilterBuffer;
	delete outshader;
	particlesys.reset();
	skyboxShader.reset();
	delete shadowrender;
	delete RelfectionBuffer;
}

void ForwardRenderer::FinaliseRender()
{
	RenderFitlerBufferOutput();
}

