#include "Shader_Main.h"
#include "RHI/RHI.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../D3D12/D3D12CBV.h"
Shader_Main::Shader_Main()
{
	//Initialise OGL shader
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	if (RHI::GetType() != RenderSystemD3D12)
	{
		m_Shader->AttachAndCompileShaderFromFile("Main_vs", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("Main_fs", SHADER_FRAGMENT);
	}
	else
	{
		m_Shader->AttachAndCompileShaderFromFile("Main_vs_12", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("Main_fs_12", SHADER_FRAGMENT);
	}

	m_Shader->BindAttributeLocation(0, "pos");
	m_Shader->BindAttributeLocation(1, "Normal");
	m_Shader->BindAttributeLocation(2, "texCoords");

	//	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

	if (RHI::GetType() == RenderSystemOGL)
	{
		m_uniform_model = glGetUniformLocation(m_Shader->GetProgramHandle(), "model");
		m_uniform_View = glGetUniformLocation(m_Shader->GetProgramHandle(), "view");
		m_UniformMVP = glGetUniformLocation(m_Shader->GetProgramHandle(), "projection");

		Uniform_Cam_Pos = glGetUniformLocation(m_Shader->GetProgramHandle(), "viewPos");
		m_uniform_texture = glGetUniformLocation(m_Shader->GetProgramHandle(), "albedoMap");
		m_uniform_LightNumber = glGetUniformLocation(m_Shader->GetProgramHandle(), "numLights");
		glUniform1i(m_uniform_texture, ALBEDOMAP);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "NormalMap"), NORMALMAP);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowcubemap2"), SHADOWCUBEMAP2);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowcubemap"), SHADOWCUBEMAP);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowdirmap"), SHADOWDIRMAP1);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Displacementmap"), DISPMAP);
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Reflectmap"), 10);
		m_IsMapUniform = glGetUniformLocation(m_Shader->GetProgramHandle(), "isMap");
		m_FarPlane = glGetUniformLocation(m_Shader->GetProgramHandle(), "far_plane");
		m_MV33 = glGetUniformLocation(m_Shader->GetProgramHandle(), "MV3x3");
		IsReflect = glGetUniformLocation(m_Shader->GetProgramHandle(), "IsReft");

		//		unsigned int block_index = glGetUniformBlockIndex(m_Shader->GetProgramHandle(), "UBuffer");
		GLuint binding_point_index = 1;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, ubo);
		//glNamedBufferData(ubo, sizeof(ConstBuffer), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		//block_index = glGetUniformBlockIndex(m_Shader->GetProgramHandle(), "UBuffer");
		//GLuint binding_point_index = 1;
		//glGenBuffers(1, &ubo);
		//glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		//glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, ubo);
		////glNamedBufferData(ubo, sizeof(ConstBuffer), nullptr, GL_DYNAMIC_DRAW);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	for (int i = 0; i < MaxConstant; i++)
	{
		SceneBuffer.push_back(SceneConstantBuffer());
	}


	GameObjectTransformBuffer = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	GameObjectTransformBuffer->CreateConstantBuffer(sizeof(SceneConstantBuffer), MaxConstant);
	CLightBuffer = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	CLightBuffer->CreateConstantBuffer(sizeof(LightBufferW), 1);
	CMVBuffer = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	CMVBuffer->CreateConstantBuffer(sizeof(MVBuffer), 1);
}
Shader_Main::~Shader_Main()
{

}
std::vector<Shader::VertexElementDESC> Shader_Main::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

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
void Shader_Main::SetNormalState(bool t, bool dispstate, bool refelction)
{
	if (RHI::GetType() == RenderSystemOGL)
	{
		if (t)
		{
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "HasNormal"), 1);
		}
		else
		{
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "HasNormal"), 0);
		}
		//default value IS ALWAYS zero!HasDisp
		if (dispstate)
		{
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "HasDisp"), 1);
		}
		else
		{
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "HasDisp"), 0);
		}
		if (refelction)
		{
			glUniform1i(IsReflect, 1);
		}
		else
		{
			glUniform1i(IsReflect, 0);
		}
	}
}

void Shader_Main::UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights)
{


	if (shadowvisstate)
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "VisShadow"), 1);
	}
	else
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "VisShadow"), 0);
	}
	if (enabledFullBright)
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "FULLBRIGHT"), 1);
	}
	else
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "FULLBRIGHT"), 0);
	}

	if (ISWATER)
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "IsWater"), 1);
		glUniform1f(glGetUniformLocation(m_Shader->GetProgramHandle(), "scroll"), currentnumber);
	}
	else
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "IsWater"), 0);
	}
	if (IsPhysics)
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "isMap"), 1);
	}
	else
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "isMap"), 0);
	}

	glUniform1i(m_uniform_LightNumber, static_cast<GLuint>(lights.size()));
	glUniform1f(m_FarPlane, static_cast<GLfloat>(ShadowFarPlane));
	glUniform1i(m_IsMapUniform, 0);
	if (!Once)
	{
		for (int i = 0; i < lights.size(); i++)
		{
			//todo: improve this!
			glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].position").c_str()), lights[i]->GetPosition().x, lights[i]->GetPosition().y, lights[i]->GetPosition().z);
			glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].Color").c_str()), lights[i]->GetColor().x, lights[i]->GetColor().y, lights[i]->GetColor().z);
			glUniform3fv(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].Direction").c_str()), 1, glm::value_ptr(lights[i]->GetDirection()));
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].type").c_str()), static_cast<int>(lights[i]->GetType()));
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].ShadowID").c_str()), (lights[i]->GetShadowId()));
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].DirShadowID").c_str()), (lights[i]->DirectionalShadowid));
			glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].HasShadow").c_str()), (lights[i]->GetDoesShadow()));
		}
		Once = true;
	}
	UBuffer.M = t->GetModel();
	UBuffer.V = c->GetView();
	UBuffer.P = c->GetProjection();
	if (!GPUStateCache::CheckCurrentUniformBuffer(ubo))
	{
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		GPUStateCache::UpdateCurrentUniformBuffer(ubo);
	}
	glBufferData(GL_UNIFORM_BUFFER, sizeof(MVPStruct), &UBuffer, GL_DYNAMIC_DRAW);
}

void Shader_Main::UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	//UBuffer
	UBuffer.M = t->GetModel();
	UBuffer.V = c->GetView();
	UBuffer.P = c->GetProjection();
#if BUILD_D3D11
	RHI::GetD3DContext()->UpdateSubresource(RHI::instance->m_constantBuffer, 0, NULL, &UBuffer, 0, 0);
	RHI::GetD3DContext()->VSSetConstantBuffers(0, 1, &RHI::instance->m_constantBuffer);
#endif
}

void Shader_Main::ClearBuffer()
{
	SceneBuffer.empty();
}

void Shader_Main::UpdateCBV()
{
	for (int i = 0; i < MaxConstant; i++)
	{
		GameObjectTransformBuffer->UpdateConstantBuffer(&SceneBuffer[i], i);
	}
}
void Shader_Main::UpdateUnformBufferEntry(const SceneConstantBuffer &bufer, int index)
{
	if (index < MaxConstant)
	{
		SceneBuffer[index] = bufer;
	}
}
void Shader_Main::SetActiveIndex(RHICommandList* list, int index)
{
	list->SetConstantBufferView(GameObjectTransformBuffer, index, D3D12CBV::MainCBV);
}
void Shader_Main::GetMainShaderSig(std::vector<Shader::ShaderParameter>& out)
{
	out.resize(6);
	out[0] = ShaderParameter(ShaderParamType::SRV, 0, 0);
	out[1] = ShaderParameter(ShaderParamType::CBV, 1, 0);
	out[2] = ShaderParameter(ShaderParamType::CBV, 2, 1);
	out[3] = ShaderParameter(ShaderParamType::CBV, 3, 2);
	//two shadows
	out[4] = ShaderParameter(ShaderParamType::SRV, 4, 1);
	out[5] = ShaderParameter(ShaderParamType::SRV, 5, 2);
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
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

void Shader_Main::UpdateMV(glm::mat4 View, glm::mat4 Projection)
{
	MV_Buffer.V = View;
	MV_Buffer.P = Projection;
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

SceneConstantBuffer Shader_Main::CreateUnformBufferEntry(Transform * t)
{
	SceneConstantBuffer m_constantBufferData;
	m_constantBufferData.M = t->GetModel();
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
			float znear = 1;
			float zfar = 500.0f;
			glm::mat4 proj = glm::perspectiveRH<float>(glm::radians(90.0f), 1.0f, znear, zfar);
			lights[i]->Projection = proj;
		}
		LightsBuffer.Light[i] = newitem;
	}
	CLightBuffer->UpdateConstantBuffer(&LightsBuffer, 0);
}

void Shader_Main::BindLightsBuffer(RHICommandList*  list)
{
	list->SetConstantBufferView(CLightBuffer, 0, D3D12CBV::LightCBV);
	list->SetConstantBufferView(CMVBuffer, 0, D3D12CBV::MPCBV);
}