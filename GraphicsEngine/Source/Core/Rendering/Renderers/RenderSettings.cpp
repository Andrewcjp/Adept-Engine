#include "RenderSettings.h"
#include "Core/BaseWindow.h"
#include "Rendering/Core/SceneRenderer.h"
#include "RHI/DeviceContext.h"
#include "Core/Maths/Math.h"
ConsoleVariable GraphSet("rg", -1, ECVarType::LaunchOnly);

RenderSettings::RenderSettings()
{
	//SetRes(BBTestMode::UHD);

	RenderScale = 1.0f;
	MaxRenderScale = 2.0f;
	ShadowSettings.UseGeometryShaderForShadows = true;
	//ShadowSettings.UseViewInstancingForShadows = true; 
	DRSSettings.EnableDynamicResolutionScaling = true;
	RTSettings.Enabled = false;
	//EnableGPUParticles = false;
	VRHMDMode = EVRHMDMode::Disabled;

	SelectedGraph = EBuiltinRenderGraphs::ForwardRenderer;

	CurrentDebug = ERenderDebugOutput::Off;
	VRXSet.EnableVRS = false;
	VRXSet.EnableVRR = false;
	AllowMeshInstancing = true;
	//ShouldRunGPUTests = true;
	if (GraphSet.GetIntValue() >= 0 && GraphSet.GetIntValue() < EBuiltinRenderGraphs::Limit)
	{
		SelectedGraph = (EBuiltinRenderGraphs::Type)GraphSet.GetIntValue();
	}
}

void RenderSettings::ValidateSettings()
{
	if (VRHMDMode != EVRHMDMode::Disabled)
	{
		RTSettings.Enabled = false;
	}
#if !RHI_SUPPORTS_RT
	//until software version is ready.
	RTSettings.Enabled = false;
#endif
}

const VRXSettings& RenderSettings::GetVRXSettings()
{
	return RHI::GetRenderSettings()->VRXSet;
}

void RenderSettings::MaxSupportedCaps(CapabilityData& MaxData)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		const CapabilityData& Data = RHI::GetDeviceContext(i)->GetCaps();
		MaxData.RTSupport = Math::Max(MaxData.RTSupport, Data.RTSupport);
		MaxData.VRSSupport = Math::Max(MaxData.VRSSupport, Data.VRSSupport);
		MaxData.SupportsViewInstancing = Math::Max(MaxData.SupportsViewInstancing, Data.SupportsViewInstancing);
	}
}

void RenderSettings::ValidateForAPI(ERenderSystemType system)
{
	if (system == ERenderSystemType::RenderSystemVulkan)
	{
		SetRes(BBTestMode::HD);
		//SelectedGraph = EBuiltinRenderGraphs::Fallback;
		RTSettings.Enabled = false;
		DRSSettings.EnableDynamicResolutionScaling = false;
		//EnableGPUParticles = false;
		VRHMDMode = EVRHMDMode::Disabled;
		AllowMeshInstancing = false;
	}
}

RendererSettings & RenderSettings::GetSettingsForRender()
{
	return RSettings;
}

void RenderSettings::SetRes(BBTestMode::Type Mode)
{
	LockBackBuffer = true;
	switch (Mode)
	{
		case BBTestMode::HD:
			LockedWidth = 1920;
			LockedHeight = 1080;
			break;
		case BBTestMode::QHD:
			LockedWidth = 2560;
			LockedHeight = 1440;
			break;
		case BBTestMode::UHD:
			LockedWidth = 3840;
			LockedHeight = 2160;
			break;
		case BBTestMode::Limit:
			LockBackBuffer = false;
			break;
	}
}

RenderConstants::RenderConstants()
{
	MAX_DYNAMIC_POINT_SHADOWS = 4;
	//	if (RHI::IsVulkan())
	{
		MAX_DYNAMIC_POINT_SHADOWS = 1;
	}
	MAX_DYNAMIC_DIRECTIONAL_SHADOWS = 1;
	MAX_LIGHTS = 16;
	DEFAULT_COPYLIST_POOL_SIZE = 4;
	MAX_LIGHTS = glm::min(MAX_POSSIBLE_LIGHTS, MAX_LIGHTS);
	LIGHTCULLING_TILE_SIZE = 16;
	MAX_MESH_INSTANCES = 50;
}

std::string RenderSettings::ToString(BBTestMode::Type t)
{
	switch (t)
	{
		case BBTestMode::HD:
			return "HD 1080P";
		case BBTestMode::QHD:
			return "QHD 1440P";
		case BBTestMode::UHD:
			return "UHD 2160P";
	}
	return "?";
}

ERenderDebugOutput::Type RenderSettings::GetDebugRenderMode()
{
	return CurrentDebug;
}

void RenderSettings::SetDebugRenderMode(ERenderDebugOutput::Type mode)
{
	CurrentDebug = mode;
}

void RenderSettings::SetRenderScale(float newscale)
{
	newscale = glm::clamp(newscale, 0.01f, MaxRenderScale);
	if (RenderScale == newscale)
	{
		return;
	}
	RenderScale = newscale;
	BaseWindow::StaticResize();
}

float RenderSettings::GetCurrentRenderScale()
{
	return RenderScale;
}

bool RenderSettings::RaytracingEnabled() const
{
	return RTSettings.Enabled;
}

RayTracingSettings & RenderSettings::GetRTSettings()
{
	return RTSettings;
}

ShadowMappingSettings & RenderSettings::GetShadowSettings()
{
	return ShadowSettings;
}

DynamicResolutionSettings & RenderSettings::GetDynamicResolutionSettings()
{
	return DRSSettings;
}
