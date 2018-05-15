#include "ForwardRenderer.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"
#include "../Editor/Editor_Camera.h"
#include "../EngineGlobals.h"
#include "../PostProcessing/PostProcessing.h"
#include "../Core/Engine.h"
#include "../RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
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
}
void ForwardRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (RHI::IsD3D12())
	{
		if (D3D12RHI::Instance)
		{
			D3D12RHI::Instance->ResizeSwapChain(width, height);
		}
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

	RenderSkybox();

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
	mainshader = new Shader_Main();
	if (RHI::GetType() == RenderSystemOGL)
	{
		QuerryShader = new Shader_Querry();
	}
	FilterBuffer = RHI::CreateFrameBuffer(m_width, m_height,nullptr, FrameBufferRatio);
	//outshader = new ShaderOutput(FilterBuffer->GetWidth(), FilterBuffer->GetHeight());
	outshader = nullptr;
	//RelfectionBuffer = RHI::CreateFrameBuffer(ReflectionBufferWidth, ReflectionBufferHeight);
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
			D3D12RHI::Instance->ExecSetUpList();
			once = false;
		}
	}

	if (mainscene->StaticSceneNeedsUpdate)
	{
		shadowrender->InitShadows(*Lights, ShadowCMDList);
		shadowrender->Renderered = false;
		mainshader->UpdateLightBuffer(*Lights);
		PrepareData();
		mainshader->UpdateCBV();
	}
	
	MainCommandList->ResetList();
	
	D3D12TimeManager::Instance->StartTimer(MainCommandList);
	MainCommandList->SetScreenBackBufferAsRT();
	MainCommandList->ClearScreen();	
	mainshader->UpdateMV(MainCamera);	
	mainshader->BindLightsBuffer(MainCommandList);
	
	if (false)
	{
		ShadowCMDList->ResetList();
		mainshader->BindLightsBuffer(ShadowCMDList);

		shadowrender->RenderShadowMaps(MainCamera, (*Lights), (*Objects), ShadowCMDList, mainshader);
		ShadowCMDList->Execute();

		shadowrender->BindShadowMapsToTextures(MainCommandList);
	}
	mainshader->UpdateMV(MainCamera);
	MainCommandList->SetRenderTarget(FilterBuffer);
	MainCommandList->ClearFrameBuffer(FilterBuffer);
	for (size_t i = 0; i < (*Objects).size(); i++)
	{
		mainshader->SetActiveIndex(MainCommandList, i);
		(*Objects)[i]->Render(false, MainCommandList);
	}
	MainCommandList->SetRenderTarget(nullptr);
	//D3D12TimeManager::Instance->EndTimer(MainCommandList);
	MainCommandList->Execute();

	Post->ExecPPStack(FilterBuffer);
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
	

}
void ForwardRenderer::RenderFitlerBufferOutput()
{
	BindAsRenderTarget();

	/*outshader->SetShaderActive();
	outshader->UpdateUniforms(nullptr, MainCamera);*/
	//FilterBuffer->BindToTextureUnit(0);
	/*outshader->RenderPlane();*/
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
	Post = new PostProcessing();
	Post->Init();
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

//void ForwardRenderer::FixedUpdatePhysx(float dtime)
//{
//	deltatime = dtime;
//	//todo: Move to Compoenent;
//	//todo: Update Objects
//	if (RenderGrass)
//	{
//		grasstest->UpdateAnimation(deltatime);
//	}
//	if (LoadParticles && RenderParticles)
//	{
//		particlesys->Add(deltatime);
//		particlesys->Simulate(deltatime, MainCamera->GetPosition());
//	}
//}

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
	/*delete FilterBuffer;*/
	delete outshader;
	particlesys.reset();
	skyboxShader.reset();
//	delete shadowrender;
	//delete RelfectionBuffer;
}

void ForwardRenderer::FinaliseRender()
{
	RenderFitlerBufferOutput();
}

