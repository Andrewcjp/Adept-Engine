
#include "Shader_EnvMap.h"
#include "Rendering/Core/Mesh.h"
#include "RHI/RHI_inc.h"
#include "../../Core/SceneRenderer.h"

IMPLEMENT_GLOBAL_SHADER(Shader_EnvMap);
Shader_EnvMap::Shader_EnvMap(class DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("CubeMap_Convolute_IntergrateBRDF_fs", EShaderType::SHADER_FRAGMENT);
}


Shader_EnvMap::~Shader_EnvMap()
{
	EnqueueSafeRHIRelease(CmdList);
	EnqueueSafeRHIRelease(ShaderData);
	EnqueueSafeRHIRelease(CubeBuffer);
	EnqueueSafeRHIRelease(Cube);
	EnqueueSafeRHIRelease(EnvBRDFBuffer);
}

void Shader_EnvMap::Init()
{
	const int MaxMipLevels = 4;
	const int Size = 128;
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateCubeColourDepth(Size, Size);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	Desc.MipCount = MaxMipLevels;//generate mips for Each level of reflection
	CubeBuffer = RHI::CreateFrameBuffer(Device, Desc);
	Desc = RHIFrameBufferDesc::CreateColour(Size, Size);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	EnvBRDFBuffer = RHI::CreateFrameBuffer(Device, Desc);
	CmdList = RHI::CreateCommandList(ECommandListType::Graphics, Device);
	RHIPipeLineStateDesc desc;
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = this;
	desc.RenderTargetDesc = EnvBRDFBuffer->GetPiplineRenderDesc();
	CmdList->SetPipelineStateDesc(desc);
	ShaderData = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
	ShaderData->CreateConstantBuffer(sizeof(SData) * 6, 6);
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f,  0.0f), glm::vec3(0.0f,  0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	for (int i = 0; i < 6; i++)
	{
		Data[i].faceID = i;
		Data[i].VP = captureProjection * captureViews[i];
		ShaderData->UpdateConstantBuffer(&Data[i], i);
	}

	MeshLoader::FMeshLoadingSettings set;
	set.Scale = glm::vec3(0.1f);
	set.InitOnAllDevices = true;
	Cube = RHI::CreateMesh("models\\SkyBoxCube.obj", set);
}

void Shader_EnvMap::ProcessTexture(BaseTextureRef Target)
{
	CmdList->ResetList();
	CmdList->SetTexture(Target, 0);
	CmdList->BeginRenderPass(RHIRenderPassDesc(CubeBuffer));
	for (int i = 0; i < 6; i++)
	{
		CmdList->SetConstantBufferView(ShaderData, i, 1);
		Cube->Render(CmdList, true);
	}
	CmdList->EndRenderPass();
	CmdList->Execute();
}

void Shader_EnvMap::ComputeEnvBRDF()
{
	CmdList->ResetList();
	RHIRenderPassDesc D = RHIRenderPassDesc(EnvBRDFBuffer);
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	CmdList->BeginRenderPass(D);
	CmdList->SetConstantBufferView(ShaderData, 0, 0);
	SceneRenderer::DrawScreenQuad(CmdList);
	CmdList->EndRenderPass();
	EnvBRDFBuffer->SetResourceState(CmdList, EResourceState::PixelShader);
	CmdList->Execute();
}

std::vector<ShaderParameter> Shader_EnvMap::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, 0));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_EnvMap::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out[0].Stride = sizeof(glm::vec4);
	return out;
}
