#include "LightCullingEngine.h"
#include "Core/Assets/Scene.h"
#include "Core/BaseWindow.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/StringUtil.h"
#include "Rendering/Core/Culling/CullingManager.h"
#include "Rendering/Core/SceneRenderer.h"

#include "Rendering/Shaders/Culling/Shader_LightCull.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "RHI/DeviceContext.h"
#include "../Screen.h"

static ConsoleVariable ShowLightBounds("c.ShowLightBounds", 0);
static ConsoleVariable FreezeLightCulling("c.LightFreeze", 0);
const std::string LightVisible = "CPU Lights Visible";
LightCullingEngine::LightCullingEngine()
{
	PerfManager::Get()->AddTimer(LightVisible.c_str(), "Culling");
}
LightCullingEngine::~LightCullingEngine()
{
	EnqueueSafeRHIRelease(LightCullingBuffer);
}

void LightCullingEngine::Init(CullingManager * m)
{
	if (CullingList[0] == nullptr)
	{
		CullingList[0] = RHI::CreateCommandList(ECommandListType::Compute);
		if (RHI::SupportVR())
		{
			CullingList[1] = RHI::CreateCommandList(ECommandListType::Compute);
		}
	}
	CreateLightDataBuffer();
	Manager = m;
	LightDataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	RHIBufferDesc Desc = {};
	Desc.CreateSRV = true;
	Desc.ElementCount = MAX_POSSIBLE_LIGHTS;
	Desc.Stride = sizeof(LightUniformBuffer);
	Desc.Accesstype = EBufferAccessType::Dynamic;
	LightDataBuffer->CreateBuffer(Desc);
}


void LightCullingEngine::LaunchCullingForScene(RHICommandList* list, EEye::Type Eye)
{

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_LightCull>());
	list->SetPipelineStateDesc(desc);
	list->SetUAV(LightCullingBuffer, "DstTexture");
	SceneRenderer::Get()->BindLightsBuffer(list, desc.ShaderInUse->GetSlotForName("LightBuffer"));
	SceneRenderer::Get()->BindMvBuffer(list, desc.ShaderInUse->GetSlotForName("CameraData"));
	//LightDataBuffer->BindBufferReadOnly(list, desc.ShaderInUse->GetSlotForName("LightList"));
	list->SetBuffer(LightDataBuffer, "LightList");
	list->Dispatch(GetLightGridDim().x, GetLightGridDim().y, 1);
}

glm::ivec2 LightCullingEngine::GetLightGridDim()
{
	const int TileSize = RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE;
	int tileWidth = glm::iround(glm::ceil(Screen::GetScaledWidth() / TileSize));
	int tileHeight = glm::iround(glm::ceil(Screen::GetScaledHeight() / TileSize));
	tileHeight += 1;
	return glm::ivec2(tileWidth, tileHeight);
}

void LightCullingEngine::WaitForCulling(RHICommandList * list)
{}

void LightCullingEngine::BindLightBuffer(RHICommandList* list, bool deferred /*= false*/)
{
	if (deferred)
	{
		LightCullingBuffer->SetBufferState(list, EBufferResourceState::Read);
		//LightCullingBuffer->BindBufferReadOnly(list, DeferredLightingShaderRSBinds::LightBuffer);
		LightDataBuffer->BindBufferReadOnly(list, DeferredLightingShaderRSBinds::LightDataBuffer);
	}
	else
	{
		LightDataBuffer->BindBufferReadOnly(list, DeferredLightingShaderRSBinds::LightDataBuffer);
	}
}

RHIBuffer* LightCullingEngine::GetLightDataBuffer()
{
	return LightDataBuffer;
}

void LightCullingEngine::Unbind(RHICommandList* list)
{
	//LightBuffer->SetBufferState(list, EBufferResourceState::UnorderedAccess);
}


void LightCullingEngine::RunLightBroadphase()
{

	//Run a sphere to sphere test
	std::vector<Light*> lights = BaseWindow::GetScene()->GetLights();
	if (!FreezeLightCulling.GetBoolValue())	
	{    
		LightsInFustrum.clear();
		for (int i = 0; i < lights.size(); i++)
		{
			Light* L = lights[i];
			if (Manager->GetFustrum()->SphereInFrustum(L->GetPosition(), L->GetRange()))
			{
				LightsInFustrum.push_back(L);
			}
		}
		PerfManager::AddToCountTimer(LightVisible, LightsInFustrum.size());
	}
	//then check against the fustrum
	if (ShowLightBounds.GetBoolValue())
	{
		for (int i = 0; i < lights.size(); i++)
		{
			Light* L = lights[i];
			glm::vec3 Col = VectorUtils::Contains(LightsInFustrum, L) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

			DebugDrawers::DrawDebugSphere(L->GetPosition(), 0.5f, Col);
			if (L->GetType() == ELightType::Point)
			{
				DebugDrawers::DrawDebugSphere(L->GetPosition(), L->GetRange(), L->GetColor());
			}
			//#LCULLING: Support other light types
		}
	}	
	UpdateLightsBuffer();
}

void LightCullingEngine::BuildLightWorldGrid()
{}

void LightCullingEngine::UpdateLightsBuffer()
{
	LightData.clear();
	for (Light* L : LightsInFustrum)
	{
		LightUniformBuffer newitem = SceneRenderer::CreateLightEntity(L, 0);	
		LightData.push_back(newitem);
	}
	LightDataBuffer->UpdateBufferData(LightData.data(), sizeof(LightUniformBuffer)*LightData.size(), EBufferResourceState::Read);
}

void LightCullingEngine::Resize()
{
	CreateLightDataBuffer();
}

int LightCullingEngine::GetNumLights() const
{
	return LightsInFustrum.size();
}

std::vector<Light*> LightCullingEngine::GetCurrentlyRelevantLights()
{
	return LightsInFustrum;
}

void LightCullingEngine::CreateLightDataBuffer()
{
	if (LightCullingBuffer != nullptr)
	{
		//return;
		EnqueueSafeRHIRelease(LightCullingBuffer);
	}
	RHIBufferDesc desc;
	desc.AllowUnorderedAccess = true;
	desc.CreateUAV = true;
	desc.CreateSRV = true;
	desc.Stride = sizeof(uint);
	desc.ElementCount = LightCullingEngine::GetLightGridDim().x * LightCullingEngine::GetLightGridDim().y * (RHI::GetRenderConstants()->MAX_LIGHTS);
	desc.Accesstype = EBufferAccessType::GPUOnly;
	LightCullingBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	LightCullingBuffer->CreateBuffer(desc);
	Log::LogMessage("Light culling buffer is " + StringUtils::ToString((desc.Stride*desc.ElementCount) / 1e6) + "MB");
}
