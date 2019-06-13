#include "LightCullingEngine.h"
#include "Core/BaseWindow.h"
#include "Core/Utils/StringUtil.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Rendering/Shaders/Culling/Shader_LightCull.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "RHI/DeviceContext.h"
#include "Core/Assets/Scene.h"
#include "Core/Utils/DebugDrawers.h"
static ConsoleVariable ShowLightBounds("c.ShowLightBounds", 0);
LightCullingEngine::LightCullingEngine()
{}


LightCullingEngine::~LightCullingEngine()
{
	EnqueueSafeRHIRelease(LightBuffer);
}

void LightCullingEngine::Init()
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
}

void LightCullingEngine::LaunchCullingForScene(EEye::Type Eye)
{
	//return;
	RHICommandList* list = CullingList[Eye];
	list->ResetList();
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_LightCull>());
	list->SetPipelineStateDesc(desc);
	list->SetUAV(LightBuffer->GetUAV(), "DstTexture");
	BaseWindow::GetCurrentRenderer()->SceneRender->BindLightsBuffer(list, desc.ShaderInUse->GetSlotForName("LightBuffer"));

	list->Dispatch(GetLightGridDim().x, GetLightGridDim().y, 1);
	list->Execute();
	RHI::GetDefaultDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

glm::ivec2 LightCullingEngine::GetLightGridDim()
{
	const int TileSize = RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE;
	int tileWidth = glm::ceil(BaseWindow::GetCurrentRenderer()->GetScaledWidth() / TileSize);
	int tileHeight = glm::ceil(BaseWindow::GetCurrentRenderer()->GetScaledHeight() / TileSize);
	tileHeight += 1;
	return glm::ivec2(tileWidth, tileHeight);
}

void LightCullingEngine::WaitForCulling(RHICommandList * list)
{}

void LightCullingEngine::BindLightBuffer(RHICommandList * list)
{
	//#TODO: the GPU hangs if bound as a structured buffer 
#if 0
	LightBuffer->SetBufferState(list, EBufferResourceState::Read);
	LightBuffer->BindBufferReadOnly(list, MainShaderRSBinds::LightBuffer);
#else
	LightBuffer->GetUAV()->Bind(list, MainShaderRSBinds::LightBuffer);
#endif
}
void LightCullingEngine::Unbind(RHICommandList* list)
{
	//LightBuffer->SetBufferState(list, EBufferResourceState::UnorderedAccess);
}


void LightCullingEngine::RunLightBroadphase()
{
	//Run a sphere to sphere test

	//then check against the fustrum
	if (ShowLightBounds.GetBoolValue())
	{
		std::vector<Light*> lights = BaseWindow::GetScene()->GetLights();

		for (int i = 0; i < lights.size(); i++)
		{
			Light* L = lights[i];
			DebugDrawers::DrawDebugSphere(L->GetPosition(), 0.5f, L->GetColor());
			if (L->GetType() == ELightType::Point)
			{
				DebugDrawers::DrawDebugSphere(L->GetPosition(), L->GetRange(), L->GetColor());
			}			
			//#LCULLING: Support other light types
		}
	}
}


void LightCullingEngine::CreateLightDataBuffer()
{
	if (LightBuffer != nullptr)
	{
		//return;
		EnqueueSafeRHIRelease(LightBuffer);
	}
	RHIBufferDesc desc;
	desc.AllowUnorderedAccess = true;
	desc.CreateUAV = true;
	desc.CreateSRV = true;
	desc.Stride = sizeof(uint);
	desc.ElementCount = LightCullingEngine::GetLightGridDim().x * LightCullingEngine::GetLightGridDim().y*(RHI::GetRenderConstants()->MAX_LIGHTS);
	desc.Accesstype = EBufferAccessType::GPUOnly;
	LightBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
	LightBuffer->CreateBuffer(desc);
	Log::LogMessage("Light culling buffer is " + StringUtils::ToStringFloat((desc.Stride*desc.ElementCount) / 1e6) + "MB");
}
