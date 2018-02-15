#include "stdafx.h"
#include "Shader_UIBatch.h"
#include "../D3D12/D3D12CBV.h"
#include "../D3D12/D3D12Shader.h"
Shader_UIBatch::Shader_UIBatch()
{

	m_Shader = RHI::CreateShaderProgam();
	if (m_Shader == nullptr)
	{
		return;
	}
	m_Shader->CreateShaderProgram();

	m_Shader->AttachAndCompileShaderFromFile("UI_Batch_vs", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("UI_Batch_fs", SHADER_FRAGMENT);

	m_Shader->BindAttributeLocation(0, "vertex");

	if (RHI::IsD3D12())
	{
		D3D12Shader* sh = (D3D12Shader*)m_Shader;
		///(float2 position : POSITION, int back : NORMAL0, float3 fc : TEXCOORD, float3 bc : TEXCOORD1)
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32_UINT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		sh->SetInputDesc(inputElementDescs, 4);
		sh->DepthTest = false;
	}
	//	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	//};
	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
	if (RHI::IsD3D12())
	{
		CBV = new D3D12CBV();
		CBV->InitCBV(sizeof(UnifromData));		
	}
}


Shader_UIBatch::~Shader_UIBatch()
{}


void Shader_UIBatch::PushTOGPU(CommandListDef* list)
{
	CBV->SetDescriptorHeaps(list);
	CBV->SetGpuView(list, 0);

}
void Shader_UIBatch::UpdateUniforms(glm::mat4x4 Proj)
{
	if (RHI::IsD3D12())
	{
		data.Proj = Proj;
		CBV->UpdateCBV(data, 0);

		//CBV->SetGpuView
	}
	else if (RHI::IsOpenGL())
	{
		glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(Proj));
	}

}

D3D12Shader * Shader_UIBatch::GetD3D12Shader()
{
	if (RHI::IsD3D12())
	{
		return (D3D12Shader*)m_Shader;
	}
	return nullptr;
}
