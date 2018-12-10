#include "Shader_SkeletalMesh.h"
#include "RHI/Shader.h"
#include "Shader_Main.h"
#include "Core/Platform/PlatformCore.h"

DECLARE_GLOBAL_SHADER(Shader_SkeletalMesh);
Shader_SkeletalMesh::Shader_SkeletalMesh(DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam(dev);
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_DIR_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)));
	m_Shader->AttachAndCompileShaderFromFile("Anim_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Gen\\Test2", EShaderType::SHADER_FRAGMENT);
	BonesBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	BonesBuffer->CreateConstantBuffer(sizeof(BoneData), 1);
}
//Anim_vs

Shader_SkeletalMesh::~Shader_SkeletalMesh()
{
	EnqueueSafeRHIRelease(BonesBuffer);
}

std::vector<Shader::ShaderParameter> Shader_SkeletalMesh::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Shader_Main::GetMainShaderSig(Output);
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 9, 5));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_SkeletalMesh::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> foamt = Shader_Main::GetMainVertexFormat();//ends on 32 
	foamt.push_back(VertexElementDESC{ "BLENDINDICES", 0, FORMAT_R32G32B32A32_UINT, 0, 44,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	foamt.push_back(VertexElementDESC{ "TEXCOORD", 2, FORMAT_R32G32B32A32_FLOAT, 0, 44 + 12,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return foamt;
}

void Shader_SkeletalMesh::PushBones(std::vector<glm::mat4x4>& bonetrans, RHICommandList* list)
{
	ensure(bonetrans.size() < MAX_BONES);
	if (bonetrans.size() == 0)
	{
		return;
	}
	for (int i = 0; i < bonetrans.size(); i++)
	{
		boneD.Bones[i] = bonetrans[i];
	}
	BonesBuffer->UpdateConstantBuffer(&boneD, 0);
	list->SetConstantBufferView(BonesBuffer, 0, 9);
}