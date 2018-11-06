
#include "Shader_Convolution.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHI/RHICommandList.h"
#include "RHI/BaseTexture.h"
#include "Core/Platform/PlatformCore.h"
DECLARE_GLOBAL_SHADER(Shader_Convolution);
Shader_Convolution::Shader_Convolution(class DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("CubeMap_Convoluter_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("CubeMap_Convoluter_fs", EShaderType::SHADER_FRAGMENT);

}


Shader_Convolution::~Shader_Convolution()
{
	EnqueueSafeRHIRelease(CmdList);
	EnqueueSafeRHIRelease(ShaderData);
	EnqueueSafeRHIRelease(CubeBuffer);
	EnqueueSafeRHIRelease(Cube);
	delete (Test);
}

void Shader_Convolution::init()
{
	const int Size = 1024;
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateCubeColourDepth(Size, Size);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	CubeBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	CmdList = RHI::CreateCommandList();
	CmdList->SetPipelineState(PipeLineState{ false,false,false });
	CmdList->CreatePipelineState(this, CubeBuffer);
	ShaderData = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	ShaderData->CreateConstantBuffer(sizeof(SData) * 6, 6);
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  -1.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, 1.0f), glm::vec3(0.0f, 1.0f,  0.0f))
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

void Shader_Convolution::ComputeConvolution(BaseTexture* Target)
{
	ensure(Target->GetType() == BaseTexture::ETextureType::Type_CubeMap);
	CmdList->ResetList();
	CmdList->ClearFrameBuffer(CubeBuffer);
	CmdList->SetTexture(Target, 0); 
	for (int i = 0; i < 6; i++)
	{
		CmdList->SetRenderTarget(CubeBuffer);
		CmdList->SetConstantBufferView(ShaderData, i, 1);
		//Test->RenderScreenQuad(CmdList);
		Cube->Render(CmdList);
	}
	CmdList->Execute();
}

std::vector<Shader::ShaderParameter> Shader_Convolution::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_Convolution::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

Shader_Convolution::QuadDrawer::~QuadDrawer()
{
	EnqueueSafeRHIRelease(VertexBuffer);
}

void Shader_Convolution::QuadDrawer::init()
{
	float g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
}

void Shader_Convolution::QuadDrawer::RenderScreenQuad(RHICommandList * list)
{
	//todo: less than full screen!
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive(6, 1, 0, 0);

}
