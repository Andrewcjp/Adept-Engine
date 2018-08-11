#include "Shader_Main.h"
#include "RHI/RHI.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Core/GameObject.h"
#include "Core/Utils/MemoryUtils.h"
#include <algorithm>
Shader_Main::Shader_Main(bool LoadForward)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->CreateShaderProgram();
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_DIR_SHADOWS", std::to_string(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("POINT_SHADOW_OFFSET", "t" + std::to_string(3 + RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(MAX_LIGHTS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("WITH_DEFERRED", std::to_string((int)!LoadForward)));

	m_Shader->AttachAndCompileShaderFromFile("Main_vs", SHADER_VERTEX);
	if (LoadForward)
	{
		m_Shader->AttachAndCompileShaderFromFile("Main_fs", SHADER_FRAGMENT);
	}
	else
	{
		m_Shader->AttachAndCompileShaderFromFile("DeferredWrite_fs", SHADER_FRAGMENT);
	}


	m_Shader->ActivateShaderProgram();

	for (int i = 0; i < MaxConstant; i++)
	{
		SceneBuffer.push_back(SceneConstantBuffer());
	}
	for (int i = 0; i < RHI::GetDeviceCount(); i++)//optimize!
	{
		GameObjectTransformBuffer[i] = RHI::CreateRHIBuffer(RHIBuffer::Constant, RHI::GetDeviceContext(i));
		GameObjectTransformBuffer[i]->CreateConstantBuffer(sizeof(SceneConstantBuffer), MaxConstant);
	}
	CLightBuffer = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	CLightBuffer->CreateConstantBuffer(sizeof(LightBufferW), 1, true);
	CMVBuffer = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	CMVBuffer->CreateConstantBuffer(sizeof(MVBuffer), 1, true);
}
Shader_Main::~Shader_Main()
{
	delete CLightBuffer;
	delete CMVBuffer;
	MemoryUtils::DeleteCArray(GameObjectTransformBuffer, MAX_DEVICE_COUNT);
}
std::vector<Shader::VertexElementDESC> Shader_Main::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

void Shader_Main::SetNormalVis()
{
	if (vistate)
	{
		vistate = false;
	}
	else
	{
		vistate = true;
	}
}
void Shader_Main::SetFullBright()
{
	if (enabledFullBright)
	{
		enabledFullBright = false;
	}
	else
	{
		enabledFullBright = true;
	}
}
void Shader_Main::SetShadowVis()
{
	if (shadowvisstate)
	{
		shadowvisstate = false;
	}
	else
	{
		shadowvisstate = true;
	}
}

void Shader_Main::ClearBuffer()
{
	SceneBuffer.empty();
}

void Shader_Main::UpdateCBV()
{
	for (int DeviceIndex = 0; DeviceIndex < RHI::GetDeviceCount(); DeviceIndex++)//optimize!
	{
		for (int i = 0; i < MaxConstant; i++)
		{
			GameObjectTransformBuffer[DeviceIndex]->UpdateConstantBuffer(&SceneBuffer[i], i);
		}
	}
}
void Shader_Main::UpdateUnformBufferEntry(const SceneConstantBuffer &bufer, int index)
{
	if (index < MaxConstant)
	{
		SceneBuffer[index] = bufer;
	}
}
void Shader_Main::SetActiveIndex(RHICommandList* list, int index, int DeviceIndex)
{
	list->SetConstantBufferView(GameObjectTransformBuffer[DeviceIndex], index, Shader_Main::MainCBV);
}
void Shader_Main::GetMainShaderSig(std::vector<Shader::ShaderParameter>& out)
{
	out.resize(11);
	out[0] = ShaderParameter(ShaderParamType::SRV, 0, 0);
	out[1] = ShaderParameter(ShaderParamType::CBV, 1, 0);
	out[2] = ShaderParameter(ShaderParamType::CBV, 2, 1);
	out[3] = ShaderParameter(ShaderParamType::CBV, 3, 2);
	//two shadows
	ShaderParameter parm = ShaderParameter(ShaderParamType::SRV, 4, 3);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	out[4] = parm;
	parm = ShaderParameter(ShaderParamType::SRV, 5, 3 + RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS;
	out[5] = parm;

	out[6] = ShaderParameter(ShaderParamType::SRV, 6, 1);
	out[7] = ShaderParameter(ShaderParamType::SRV, 7, 10);
	out[8] = ShaderParameter(ShaderParamType::SRV, 8, 11);
	out[9] = ShaderParameter(ShaderParamType::SRV, 9, 12);
	out[10] = ShaderParameter(ShaderParamType::SRV, 10, 13);
}

std::vector<Shader::ShaderParameter> Shader_Main::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	GetMainShaderSig(Output);
	return Output;
}

void Shader_Main::UpdateMV(Camera * c)
{
	MV_Buffer.V = c->GetView();
	MV_Buffer.P = c->GetProjection();
	MV_Buffer.CameraPos = c->GetPosition();
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

void Shader_Main::UpdateMV(glm::mat4 View, glm::mat4 Projection)
{
	//	ensure(false);
	MV_Buffer.V = View;
	MV_Buffer.P = Projection;
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

SceneConstantBuffer Shader_Main::CreateUnformBufferEntry(GameObject * t)
{
	SceneConstantBuffer m_constantBufferData;
	m_constantBufferData.M = t->GetTransform()->GetModel();
	m_constantBufferData.HasNormalMap = false;
	if (t->GetMat() != nullptr)
	{
		m_constantBufferData.HasNormalMap = t->GetMat()->HasNormalMap();
		m_constantBufferData.Metallic = t->GetMat()->GetProperties()->Metallic;
		m_constantBufferData.Roughness = t->GetMat()->GetProperties()->Roughness;
	}
	//used in the prepare stage for this frame!
	return m_constantBufferData;
}

void Shader_Main::UpdateLightBuffer(std::vector<Light*> lights)
{
	for (int i = 0; i < lights.size(); i++)
	{
		LightUniformBuffer newitem;
		newitem.position = lights[i]->GetPosition();
		newitem.color = glm::vec3(lights[i]->GetColor());
		newitem.Direction = lights[i]->GetDirection();
		newitem.type = lights[i]->GetType();
		newitem.HasShadow = lights[i]->GetDoesShadow();
		newitem.ShadowID = lights[i]->GetShadowId();
		if (lights[i]->GetType() == Light::Directional || lights[i]->GetType() == Light::Spot)
		{
			glm::mat4 LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));//world up
			glm::vec3 position = glm::vec3(0, 20, 50);
			//position = lights[i]->GetPosition();
			LightView = glm::lookAtLH<float>(position, position + newitem.Direction, glm::vec3(0, 0, 1));//world up
			float size = 100.0f;
			glm::mat4 proj;
			if (lights[i]->GetType() == Light::Spot)
			{
				proj = glm::perspective<float>(glm::radians(45.0f), 1.0f, 2.0f, 50.0f);
				LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), lights[i]->GetPosition() + newitem.Direction, glm::vec3(0, 0, 1));//world up
			}
			else
			{
				proj = glm::orthoLH<float>(-size, size, -size, size, -200, 100);
			}
			lights[i]->Projection = proj;
			lights[i]->DirView = LightView;
			newitem.LightVP = proj * LightView;
		}
		if (lights[i]->GetType() == Light::Point)
		{
			float znear = 1.0f;
			float zfar = 500;
			glm::mat4 proj = glm::perspectiveLH<float>(glm::radians(90.0f), 1.0f, znear, zfar);
			lights[i]->Projection = proj;
		}
		LightsBuffer.Light[i] = newitem;
	}
	CLightBuffer->UpdateConstantBuffer(&LightsBuffer, 0);
}

void Shader_Main::BindLightsBuffer(RHICommandList*  list, bool JustLight)
{
	list->SetConstantBufferView(CLightBuffer, 0, Shader_Main::LightCBV);
	if (!JustLight)
	{
		BindMvBuffer(list, Shader_Main::MPCBV);
	}
}

void Shader_Main::BindMvBuffer(RHICommandList * list, int slot)
{
	list->SetConstantBufferView(CMVBuffer, 0, slot);
}