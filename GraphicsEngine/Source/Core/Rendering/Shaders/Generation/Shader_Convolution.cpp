
#include "Shader_Convolution.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHI/RHICommandList.h"
#include "RHI/BaseTexture.h"
#include "Core/Platform/PlatformCore.h"
IMPLEMENT_GLOBAL_SHADER(Shader_Convolution);
Shader_Convolution::Shader_Convolution(class DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("CubeMap_Convoluter_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("CubeMap_Convoluter_fs", EShaderType::SHADER_FRAGMENT);
}


Shader_Convolution::~Shader_Convolution()
{
	EnqueueSafeRHIRelease(CmdList);
	EnqueueSafeRHIRelease(ShaderData);
	EnqueueSafeRHIRelease(CubeBuffer);
	EnqueueSafeRHIRelease(Cube);
	SafeDelete(Test);
}

void Shader_Convolution::init()
{
	const int Size = 1024;
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateCubeColourDepth(Size, Size);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	CubeBuffer = RHI::CreateFrameBuffer(Device, Desc);
	CmdList = RHI::CreateCommandList(ECommandListType::Graphics, Device);

	ConvPSODesc.InitOLD(false, false, false);
	ConvPSODesc.ShaderInUse = this;
	ConvPSODesc.FrameBufferTarget = CubeBuffer;
	CmdList->SetPipelineStateDesc(ConvPSODesc);
	ShaderData = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
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
	set.InitOnAllDevices = true;
	Cube = RHI::CreateMesh("models\\SkyBoxCube.obj", set);
}

void Shader_Convolution::ComputeConvolution(BaseTextureRef Target, FrameBuffer* Buffer)
{
	ensure(Target->GetType() == ETextureType::Type_CubeMap);
	CmdList->ResetList();
	RHIRenderPassDesc D = RHIRenderPassDesc(Buffer, ERenderPassLoadOp::Clear);
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	CmdList->BeginRenderPass(D);
	CmdList->SetTexture(Target, 0);
	for (int i = 0; i < 6; i++)
	{		
		CmdList->SetConstantBufferView(ShaderData, i, 1);
		Cube->Render(CmdList);
	}
	CmdList->EndRenderPass();
	CmdList->Execute();
}

void Shader_Convolution::ComputeConvolutionProbe(RHICommandList* List, FrameBuffer* Target, FrameBuffer * Buffer)
{		
	RHIRenderPassDesc D = RHIRenderPassDesc(Buffer, ERenderPassLoadOp::Clear);
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	List->SetPipelineStateDesc(ConvPSODesc);
	List->BeginRenderPass(D);
	List->SetFrameBufferTexture(Target, 0);
	for (int i = 0; i < 6; i++)
	{
		List->SetConstantBufferView(ShaderData, i, 1);
		Cube->Render(List);
	}
	List->EndRenderPass();
}

std::vector<ShaderParameter> Shader_Convolution::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_Convolution::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out[0].Stride = sizeof(OGLVertex);
	return out;
}

Shader_Convolution::QuadDrawer::~QuadDrawer()
{
	EnqueueSafeRHIRelease(VertexBuffer);
}

void Shader_Convolution::QuadDrawer::init(DeviceContext* dev)
{
	float g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex, dev);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
}

void Shader_Convolution::QuadDrawer::RenderScreenQuad(RHICommandList * list)
{
	//todo: less than full screen!
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive(6, 1, 0, 0);

}
