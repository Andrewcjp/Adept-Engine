#include "DeferredRenderer.h"
#include "../Rendering/Core/Mesh.h"
#include "Core/Assets/ImageLoader.h"
#include "RHI/RHI.h"
#include "../Core/Components/MeshRendererComponent.h"
void DeferredRenderer::Render()
{
////	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
	shadower->RenderShadowMaps(MainCamera, Lights, Objects);
//////	glDisable(GL_BLEND);
	GeometryPass();
	//SSAOPass();

//	shadower->BindShadowMaps();
	LightingPass();
	RenderFitlerBufferOutput();
}

void DeferredRenderer::Init()
{
	GPUStateCache::Create();
	MainCamera = new Camera(glm::vec3(0, 2, 0), 90.0f, static_cast<float>(m_width / m_height), 0.1f, 1000.0f);
	DeferredWriteShader = new Shader_WDeferred();
	DeferredShader = new Shader_Deferred();
	// = new FrameBuffer_gDeferred(m_width, m_height);
#if BUILD_OPENGL
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL);
	glDisable(GL_MULTISAMPLE);
	glClearColor(0, 0, 0, 0);
#endif
	//glEnable(GL_BLEND);
	shadower = new ShadowRenderer();
	//SSAOBuffer = new FrameBufferSSAO(m_width, m_height);
	SSAOShader = new Shader_SSAO();
	//init skybox
	skybox = new GameObject();
	//skybox->AttachComponent(new MeshRendererComponent(new Mesh("../asset/models/skybox.obj"), nullptr));*/
//	skybox->SetMesh(new Mesh("../asset/models/skybox.obj"));
//////	/*SkyboxTexture = ImageLoader::instance->loadsplitCubeMap("heh");
	//SkyboxShader = RHI::CreateShaderProgam();
	//SkyboxShader->CreateShaderProgram();
	//SkyboxShader->AttachAndCompileShaderFromFile("skybox", SHADER_VERTEX);
	//SkyboxShader->AttachAndCompileShaderFromFile("skybox", SHADER_FRAGMENT);

	//SkyboxShader->BindAttributeLocation(0, "position");

	//SkyboxShader->BuildShaderProgram();
	//SkyboxShader->ActivateShaderProgram();

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


void DeferredRenderer::AddLight(Light * l)
{
	Lights.push_back(l);
}


void DeferredRenderer::GeometryPass()
{
	//->BindBufferAsRenderTarget();
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

	RenderSkybox(true);
	//->UnBind();
}
void DeferredRenderer::RenderSkybox(bool ismain)
{
#if BUILD_OPENGL
	glDepthFunc(GL_LEQUAL);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexture);
	SkyboxShader->ActivateShaderProgram();
	glUniform1i(glGetUniformLocation(SkyboxShader->GetProgramHandle(), "skybox"), 2);
	glDepthMask(GL_FALSE);
	glm::mat4 view;
	view = glm::mat4(glm::mat3(MainCamera->GetView()));
	glUniformMatrix4fv(glGetUniformLocation(SkyboxShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(MainCamera->GetProjection()));
	glUniformMatrix4fv(glGetUniformLocation(SkyboxShader->GetProgramHandle(), "view"), 1, GL_FALSE, glm::value_ptr(view));
	skybox->Render();
	glDepthMask(GL_TRUE);
#endif
}
void DeferredRenderer::SSAOPass()
{
	//SSAOBuffer->BindBufferAsRenderTarget();
	//glClear(GL_COLOR_BUFFER_BIT);
	//->BindToTextureUnit();
	SSAOShader->SetShaderActive();
	SSAOShader->UpdateUniforms(nullptr, MainCamera, Lights);
	SSAOShader->RenderPlane();
	//SSAOBuffer->UnBind();
}
void DeferredRenderer::LightingPass()
{
	FilterBuffer->BindBufferAsRenderTarget();
//	shadower->BindShadowMaps();
	//->BindToTextureUnit();
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//SSAOBuffer->BindToTextureUnit(6);
	DeferredShader->SetShaderActive();
	DeferredShader->UpdateUniforms(nullptr, MainCamera, Lights);
	DeferredShader->RenderPlane();
	//->UnbindTextures();
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
	//glClearColor(0.0f, 0.5f, 0.0f, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	outshader->SetShaderActive();
	FilterBuffer->BindToTextureUnit();
	outshader->RenderPlane();
}
void DeferredRenderer::Resize(int width, int height)
{

	m_width = width;
	m_height = height;
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)width / (float)height);
	}
	//if (// != nullptr)
	//{
	//	delete //;
	//	delete //SSAOBuffer;
	//	delete FilterBuffer;		
	//	// = new FrameBuffer_gDeferred(width, height);
	//	//SSAOBuffer = new FrameBufferSSAO(width, height);
	//	FilterBuffer = RHI::CreateFrameBuffer(width, height);
	//	outshader->Resize(width, height);
	//	SSAOShader->Resize(width, height);
	//}
}

Shader * DeferredRenderer::GetMainShader()
{
	return DeferredWriteShader;
}

std::vector<GameObject*> DeferredRenderer::GetObjects()
{
	return Objects;
}

FrameBuffer * DeferredRenderer::GetReflectionBuffer()
{
	return nullptr;
}

void DeferredRenderer::BindAsRenderTarget()
{

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
