#include "DeferredRenderer.h"
#include "../Rendering/Core/Mesh.h"
#include "Core/Assets/ImageLoader.h"
#include "OpenGL/OGLShaderProgram.h"
#include "RHI/RHI.h"
#include "../Core/Components/MeshRendererComponent.h"
void DeferredRenderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadower->RenderShadowMaps(MainCamera, Lights, Objects);
	glDisable(GL_BLEND);
	GeometryPass();
	//SSAOPass();

//	shadower->BindShadowMaps();
	LightingPass();
	RenderFitlerBufferOutput();
}

void DeferredRenderer::Init()
{
//	shadower->InitShadows(Lights);

}

void DeferredRenderer::InitOGL()
{
	GPUStateCache::Create();
	MainCamera = new Camera(glm::vec3(0, 2, 0), 90.0f, static_cast<float>(m_width / m_height), 0.1f, 1000.0f);
	DeferredWriteShader = new Shader_WDeferred();
	DeferredShader = new Shader_Deferred();
	DeferredFrameBuffer = new FrameBuffer_gDeferred(m_width, m_height);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL);
	glDisable(GL_MULTISAMPLE);
	glClearColor(0, 0, 0, 0);
	//glEnable(GL_BLEND);
	shadower = new ShadowRenderer();
	SSAOBuffer = new FrameBufferSSAO(m_width, m_height);
	SSAOShader = new Shader_SSAO();
	//init skybox
	skybox = new GameObject();
	//skybox->AttachComponent(new MeshRendererComponent(new Mesh("../asset/models/skybox.obj"), nullptr));*/
//	skybox->SetMesh(new Mesh("../asset/models/skybox.obj"));
	SkyboxTexture = ImageLoader::instance->loadsplitCubeMap("heh");
	SkyboxShader = new OGLShaderProgram();
	SkyboxShader->CreateShaderProgram();
	SkyboxShader->AttachAndCompileShaderFromFile("skybox", SHADER_VERTEX);
	SkyboxShader->AttachAndCompileShaderFromFile("skybox", SHADER_FRAGMENT);

	SkyboxShader->BindAttributeLocation(0, "position");

	SkyboxShader->BuildShaderProgram();
	SkyboxShader->ActivateShaderProgram();

	FilterBuffer = RHI::CreateFrameBuffer(m_width, m_height);
	outshader = new ShaderOutput(FilterBuffer->GetWidth(), FilterBuffer->GetHeight());
}

DeferredRenderer::~DeferredRenderer()
{
}

Camera * DeferredRenderer::GetMainCam()
{
	return MainCamera;
}

void DeferredRenderer::AddGo(GameObject * g)
{
	Objects.push_back(g);
}

void DeferredRenderer::AddPhysObj(GameObject * go)
{
	if (PhysicsObjects.size() >= MaxPhysicsObjects)
	{
		PhysicsObjects.erase(PhysicsObjects.begin());
	}
	PhysicsObjects.push_back(go);
}

void DeferredRenderer::AddLight(Light * l)
{
	Lights.push_back(l);
}

void DeferredRenderer::FixedUpdatePhysx(float dtime)
{
	//deltatime = dtime;
	for (size_t i = 0; i < PhysicsObjects.size(); i++)
	{
		PhysicsObjects[i]->FixedUpdate(dtime);
	}

}

void DeferredRenderer::GeometryPass()
{
	DeferredFrameBuffer->BindBufferAsRenderTarget();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DeferredWriteShader->SetShaderActive();
	for (size_t i = 0; i < Objects.size(); i++)
	{
		if (Objects[i]->GetReflection() == true)
		{
			continue;
		}
		DeferredWriteShader->UpdateUniforms(Objects[i]->GetTransform(), MainCamera, Lights);
		DeferredWriteShader->SetNormalState(Objects[i]->GetMat()->NormalMap != nullptr);
		Objects[i]->Render();
	}
	for (size_t i = 0; i < PhysicsObjects.size(); i++)
	{
		float distance = (fabs(glm::length(PhysicsObjects[i]->GetTransform()->GetPos() - MainCamera->GetPosition())));
		if (PhysicsObjects[i]->CheckCulled(distance, glm::angle(PhysicsObjects[i]->GetTransform()->GetPos(), MainCamera->GetForward())))
		{
			continue;
		}
		//	itemsrender++;
		DeferredWriteShader->SetNormalState((PhysicsObjects[i]->GetMat()->NormalMap != nullptr));
		//this enables non normal mapped surfaces not to be black
		DeferredWriteShader->UpdateUniforms(PhysicsObjects[i]->GetTransform(), MainCamera, Lights);
		PhysicsObjects[i]->Render();
	}
	RenderSkybox(true);
	DeferredFrameBuffer->UnBind();
	//glDisable(GL_BLEND);
}
void DeferredRenderer::RenderSkybox(bool ismain)
{
	glDepthFunc(GL_LEQUAL);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexture);
	SkyboxShader->ActivateShaderProgram();
	glUniform1i(glGetUniformLocation(SkyboxShader->GetProgramHandle(), "skybox"), 2);
	glDepthMask(GL_FALSE);
	glm::mat4 view;
	view = glm::mat4(glm::mat3(MainCamera->GetView()));
	if (ismain)
	{

	}
	else
	{
		//view = glm::mat4(glm::mat3(RefelctionCamera->GetView()));
	}
	glUniformMatrix4fv(glGetUniformLocation(SkyboxShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(MainCamera->GetProjection()));
	glUniformMatrix4fv(glGetUniformLocation(SkyboxShader->GetProgramHandle(), "view"), 1, GL_FALSE, glm::value_ptr(view));
	skybox->Render();
	glDepthMask(GL_TRUE);

}
void DeferredRenderer::SSAOPass()
{
	SSAOBuffer->BindBufferAsRenderTarget();
	glClear(GL_COLOR_BUFFER_BIT);
	DeferredFrameBuffer->BindToTextureUnit();
	SSAOShader->SetShaderActive();
	SSAOShader->UpdateUniforms(nullptr, MainCamera, Lights);
	SSAOShader->RenderPlane();
	SSAOBuffer->UnBind();
}
void DeferredRenderer::LightingPass()
{
	FilterBuffer->BindBufferAsRenderTarget();
//	shadower->BindShadowMaps();
	DeferredFrameBuffer->BindToTextureUnit();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SSAOBuffer->BindToTextureUnit(6);
	DeferredShader->SetShaderActive();
	DeferredShader->UpdateUniforms(nullptr, MainCamera, Lights);
	DeferredShader->RenderPlane();
	DeferredFrameBuffer->UnbindTextures();
	FilterBuffer->UnBind();
	//glEnable(GL_BLEND);
	//RenderSkybox(true);
	//glDisable(GL_BLEND);
}

void DeferredRenderer::ShadowPass()
{
}
void DeferredRenderer::RenderFitlerBufferOutput()
{
	BindAsRenderTarget();
	glClearColor(0.0f, 0.5f, 0.0f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	outshader->SetShaderActive();
	FilterBuffer->BindToTextureUnit();
	outshader->RenderPlane();
}
void DeferredRenderer::Resize(int width, int height)
{

	m_width = width;
	m_height = height;
	glViewport(0, 0, width, height);
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)width / (float)height);
	}
	if (DeferredFrameBuffer != nullptr)
	{
		delete DeferredFrameBuffer;
		delete SSAOBuffer;
		delete FilterBuffer;		
		DeferredFrameBuffer = new FrameBuffer_gDeferred(width, height);
		SSAOBuffer = new FrameBufferSSAO(width, height);
		FilterBuffer = RHI::CreateFrameBuffer(width, height);
		outshader->Resize(width, height);
		SSAOShader->Resize(width, height);
	}
}

Shader * DeferredRenderer::GetMainShader()
{
	return DeferredWriteShader;
}

std::vector<GameObject*> DeferredRenderer::GetObjects()
{
	return Objects;
}



void DeferredRenderer::SetReflectionCamera(Camera * )
{
}

FrameBuffer * DeferredRenderer::GetReflectionBuffer()
{
	return nullptr;
}
void DeferredRenderer::BindAsRenderTarget()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, m_width, m_height);
}

ShaderOutput * DeferredRenderer::GetFilterShader()
{
	return outshader;
}

void DeferredRenderer::DestoryRenderWindow()
{
}

void DeferredRenderer::FinaliseRender()
{
}
