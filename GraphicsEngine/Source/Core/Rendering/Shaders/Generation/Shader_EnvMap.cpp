
#include "Shader_EnvMap.h"
#include "Rendering/Core/Mesh.h"
#include "RHI/RHI_inc.h"
IMPLEMENT_GLOBAL_SHADER(Shader_EnvMap);
Shader_EnvMap::Shader_EnvMap(class DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("CubeMap_Convolute_IntergrateBRDF_fs", EShaderType::SHADER_FRAGMENT);
}


Shader_EnvMap::~Shader_EnvMap()
{
	EnqueueSafeRHIRelease(CmdList);
	delete QuadDraw;
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
	CubeBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	Desc = RHIFrameBufferDesc::CreateColour(Size, Size);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	EnvBRDFBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	QuadDraw = new Shader_Convolution::QuadDrawer();
	QuadDraw->init();
	CmdList = RHI::CreateCommandList();
	RHIPipeLineStateDesc desc;
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = this;
	desc.FrameBufferTarget = EnvBRDFBuffer;
	CmdList->SetPipelineStateDesc(desc);
	ShaderData = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
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
	Cube = RHI::CreateMesh("models\\SkyBoxCube.obj", set);
}

void Shader_EnvMap::ProcessTexture(BaseTexture * Target)
{
	CmdList->ResetList();
	CmdList->ClearFrameBuffer(CubeBuffer);
	CmdList->SetTexture(Target, 0);
	for (int i = 0; i < 6; i++)
	{
		CmdList->SetRenderTarget(CubeBuffer, 0);
		CmdList->SetConstantBufferView(ShaderData, i, 1);
		Cube->Render(CmdList, true);
	}
	CmdList->Execute();
}

void Shader_EnvMap::ComputeEnvBRDF()
{
	CmdList->ResetList();

	CmdList->ClearFrameBuffer(EnvBRDFBuffer);
	CmdList->SetRenderTarget(EnvBRDFBuffer, 0);
	CmdList->SetConstantBufferView(ShaderData, 0, 1);
	QuadDraw->RenderScreenQuad(CmdList);
	CmdList->Execute();
}

std::vector<Shader::ShaderParameter> Shader_EnvMap::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_EnvMap::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
