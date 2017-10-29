#include "ForwardRenderer.h"
#include "OpenGL/OGLShaderProgram.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"

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

	//#ifdef _DEBUG
	//		/*LoadGrass = false;
	//		RenderGrass = false;*/
	//		LoadParticles = false;
	//		RenderParticles = false;
	//#endif
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
{
}

void ForwardRenderer::Render()
{
	if (Objects == nullptr || Objects->size() == 0)
	{
		return;
	}
	ShadowPass();
	RHI::ClearColour();
	RHI::ClearDepth();
	RenderSkybox();
	if (RenderedReflection == false)
	{
		ReflectionPass();
	}
	RHI::ClearColour();
	RHI::ClearDepth();

	MainPass();
	RenderSkybox();	
}
inline std::vector<GameObject*> ForwardRenderer::InGetObj()
{
	return (*Objects);
}
void ForwardRenderer::Init()
{
	if (mainscene == nullptr)
	{
		mainscene = new Scene();
	}
	Lights = mainscene->GetLights();
	(Objects) = (mainscene->GetObjects());
	shadowrender = new ShadowRenderer();
	shadowrender->InitShadows((*Lights));
	GPUStateCache::Create();
	skybox = new GameObject();
	skybox->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/skybox.obj", GetMainShader()->GetShaderProgram(), true), nullptr));
	if (LoadGrass)
	{
		grasstest = std::make_unique<GrassPatch>();
	}

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
	shadowrender->BindShadowMaps();

	for (size_t i = 0; i < (InGetObj()).size(); i++)
	{
		if ((InGetObj())[i]->GetReflection() == true)
		{
			continue;
		}
		if ((InGetObj())[i]->UseDefaultShader == false)
		{
			grassshader->SetShaderActive();
			grassshader->UpdateUniforms((InGetObj())[i]->GetTransform(), RefelctionCamera, (*Lights));
			(InGetObj())[i]->Render();

			mainshader->SetShaderActive();
			continue;
		}
		mainshader->SetNormalState(((InGetObj())[i]->GetMat()->NormalMap != nullptr), ((InGetObj())[i]->GetMat()->DisplacementMap != nullptr), ((InGetObj())[i]->GetReflection() == true));//this enables non normal mapped surfaces not to be black

		mainshader->UpdateUniforms((InGetObj())[i]->GetTransform(), RefelctionCamera, (*Lights));
		(InGetObj())[i]->Render();
	}
	RenderSkybox(false);
	RelfectionBuffer->UnBind();
}

void ForwardRenderer::ShadowPass()
{
	shadowrender->RenderShadowMaps(MainCamera, (*Lights), (InGetObj()));
}
void ForwardRenderer::BindAsRenderTarget()
{
	RHI::BindScreenRenderTarget(m_width, m_height);
}

void ForwardRenderer::MainPass()
{
	if (RHI::GetType() == RenderSystemOGL)
	{
	//	RunQuery();
	}
	FilterBuffer->BindBufferAsRenderTarget();
	RHI::ClearColour();
	RHI::ClearDepth();
	RenderSkybox();

	mainshader->currentnumber += 0.1f* deltatime;
	mainshader->SetShaderActive();
	shadowrender->BindShadowMaps();
	int count = 0;
	for (size_t i = 0; i < (InGetObj()).size(); i++)
	{
		if (false)
		{
			GLuint passed = INT_MAX;
			GLuint available = 0;
			glGetQueryObjectuiv((InGetObj())[i]->Querry, GL_QUERY_RESULT_AVAILABLE, &available);
			if (available)
			{
				passed = 0;
				glGetQueryObjectuiv((InGetObj())[i]->Querry, GL_QUERY_RESULT, &passed);
				(InGetObj())[i]->Occluded = (passed == 0);
				(InGetObj())[i]->QuerryWait = false;
			}

			if ((InGetObj())[i]->Occluded)
			{
				count++;
				//  printf("culled %s\n ", Objects[i]->GetName());
				continue;
			}
		}
		if ((InGetObj())[i]->UseDefaultShader == false)
		{
			grassshader->SetShaderActive();
			grassshader->UpdateUniforms((InGetObj())[i]->GetTransform(), MainCamera, (*Lights));
			(InGetObj())[i]->Render();
			//	itemsrender++;
			mainshader->SetShaderActive();
			continue;
		}
		//	itemsrender++;
		mainshader->SetNormalState(((InGetObj())[i]->GetMat()->NormalMap != nullptr), ((InGetObj())[i]->GetMat()->DisplacementMap != nullptr), ((InGetObj())[i]->GetReflection() == true));//this enables non normal mapped surfaces not to be black
		if ((InGetObj())[i]->GetReflection())
		{
			mainshader->ISWATER = true;
		}
		else
		{
			mainshader->ISWATER = false;
		}
		mainshader->UpdateUniforms((InGetObj())[i]->GetTransform(), MainCamera, (*Lights));
		(InGetObj())[i]->Render();
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
}

void ForwardRenderer::RenderSkybox(bool ismain)
{

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
	FilterBuffer->BindToTextureUnit(0);
	outshader->RenderPlane();
}
void ForwardRenderer::InitOGL()
{
	MainCamera = new Camera(glm::vec3(0, 2, 0), 75.0f, static_cast<float>(m_width / m_height), 0.1f, 1000.0f);

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

