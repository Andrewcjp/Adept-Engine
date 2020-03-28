#include "RenderSettings.h"
#include "Core/BaseWindow.h"
#include "Rendering/Core/SceneRenderer.h"
#include "RHI/DeviceContext.h"
#include "Core/Maths/Math.h"
ConsoleVariable GraphSet("rg", -1, ECVarType::LaunchOnly);

RenderSettings::RenderSettings()
{
	SetRes(BBTestMode::HD);

	RenderScale = 1.0f;
	MaxRenderScale = 2.0f;
	ShadowSettings.UseGeometryShaderForShadows = true;
	//ShadowSettings.UseViewInstancingForShadows = true; 
	//DRSSettings.EnableDynamicResolutionScaling = true;
	RTSettings.Enabled = true;
	CurrentAAMode = AAMode::NONE;
	EnableGPUParticles = true;
	VRHMDMode = EVRHMDMode::Disabled;

	SelectedGraph = EBuiltinRenderGraphs::DeferredRenderer_VX_RT;

	CurrentDebug = ERenderDebugOutput::Off;
	//VRXSet.EnableVRX = true;
	VRXSet.VRXMode = EVRSMode::ForceSoftwareOnly;
	VRXSet.VRRTileSize = 16;
	AllowMeshInstancing = true;
	//ShouldRunGPUTests = true;
	if (GraphSet.GetIntValue() >= 0 && GraphSet.GetIntValue() < EBuiltinRenderGraphs::Limit)
	{
		SelectedGraph = (EBuiltinRenderGraphs::Type)GraphSet.GetIntValue();
	}
	if (SelectedGraph == EBuiltinRenderGraphs::DeferredRenderer_VX_RT)
	{
		VoxelSet.Enabled = true;
	}
	if (SelectedGraph == EBuiltinRenderGraphs::MGPU_SFR) 
	{
		CurrnetSFRSettings.Enabled = true;
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
#ifndef PLATFORM_WINDOWS
	EnableGPUParticles = false;
	VoxelSet.Enabled = false;
	//RTSettings.Enabled = false;
#endif
}

const VRXSettings& RenderSettings::GetVRXSettings()
{
	return RHI::GetRenderSettings()->VRXSet;
}

void RenderSettings::SetVRXActive(bool state)
{
	RHI::GetRenderSettings()->VRXSet.VRXActive = state;
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

VoxelSettings& RenderSettings::GetVoxelSet()
{
	return VoxelSet;
}

void RenderSettings::ValidateForAPI(ERenderSystemType system)
{
	if (system == ERenderSystemType::RenderSystemVulkan)
	{
		SetRes(BBTestMode::HD);
		//SelectedGraph = EBuiltinRenderGraphs::Fallback;
		RTSettings.Enabled = false;
		DRSSettings.EnableDynamicResolutionScaling = false;
		EnableGPUParticles = false;
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
	Testmode = Mode;
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
	MAX_DYNAMIC_DIRECTIONAL_SHADOWS = 1;
	MAX_LIGHTS = 16;
	DEFAULT_COPYLIST_POOL_SIZE = 4;
	MAX_LIGHTS = glm::min(MAX_POSSIBLE_LIGHTS, MAX_LIGHTS);
	LIGHTCULLING_TILE_SIZE = 16;
	MAX_MESH_INSTANCES = 20;
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
	CapabilityData MaxCaps = CapabilityData();
	RHI::GetRenderSettings()->MaxSupportedCaps(MaxCaps);
	if (MaxCaps.RTSupport == ERayTracingSupportType::None)
	{
		return false;
	}
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

bool VRXSettings::UseVRX(DeviceContext* con)const
{
	return UseVRS(con) || UseVRR(con);
}

bool VRXSettings::UseVRR(DeviceContext* con) const
{
	if (VRXMode == EVRSMode::HardwareOnly || !EnableVRX)
	{
		return false;
	}
	if (UseVRS(con))
	{
		return false;
	}
	return true;
}

bool VRXSettings::UseVRS(DeviceContext* con) const
{
	if (VRXMode == EVRSMode::ForceSoftwareOnly || !EnableVRX)
	{
		return false;
	}
	if (con == nullptr)
	{
		con = RHI::GetDefaultDevice();
	}
	if (con->GetCaps().VRSSupport >= EVRSSupportType::Hardware)
	{
		return true;
	}
	return false;
}
