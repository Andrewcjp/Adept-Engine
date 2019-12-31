#pragma once
#include "Core\Platform\ConsoleVariable.h"
#include "..\RenderNodes\RenderGraphSystem.h"
#include "WinLauncher.h"


class Archive;
struct CapabilityData;
namespace AAMode
{
	enum Type
	{
		NONE,
		FXAA,
		MSAA,
		SMAA,
		TAA,
	};
}


struct ERenderDebugOutput
{
	enum Type
	{
		Off,
		GBuffer_Pos,
		GBuffer_Normal,
		GBuffer_Material,
		GBuffer_RoughNess,
		GBuffer_Metallic,
		Scene_Metallic,
		Scene_RoughNess,
		Scene_LightRange,
		Scene_UVs,
		Scene_ShaderInstructionCount,
		Scene_Voxel,
		Limit
	};
};
struct RayTracingSettings
{
	bool Enabled = false;
};
struct ShadowMappingSettings
{
	bool UseGeometryShaderForShadows = false;
	bool UseViewInstancingForShadows = false;
	int MaxShadowMapSize = 4096;
	int DefaultShadowMapSize = 1024;
	int MAX_PRESAMPLED_SHADOWS = 4;
	float PreSampleBufferScale = 1.0f;
	int ViewInstancesPerDraw = 3;
};
struct DynamicResolutionSettings
{
	bool EnableDynamicResolutionScaling = false;
	int DynamicResolutionTargetFrameRate = 60;
	int DynamicResolutionTargetFrameRateVR = 95;
};
struct RendererSettings
{
	bool EnableTransparency = false;
};
struct SFRSettings
{
	//adds gaps in Render targets to show transitions
	bool DEBUG_ShowSplits = false;
	//each pixel will have a colour tint which indicates to the GPU that rendered it.
	bool DEBUG_ColourPixelsPerGPU = false;
};
namespace EVRHMDMode
{
	enum Type
	{
		Disabled,
		SteamVR,
		Debug,
		Limit
	};
}
namespace EVRSMode
{
	enum Type
	{
		HardwareAndSoftware,
		HardwareOnly,
		ForceSoftwareOnly,
		Limit
	};
}
struct VRXSettings
{
	bool EnableVRR = false;
	bool EnableVRS = false;
	EVRSMode::Type VRSMode = EVRSMode::HardwareAndSoftware;
};
//Props Are Set in the Constructor 
struct RenderSettings
{
	RenderSettings();
public:

	AAMode::Type CurrentAAMode = AAMode::NONE;
	EBuiltinRenderGraphs::Type SelectedGraph = EBuiltinRenderGraphs::DeferredRenderer;
	EBuiltInRenderGraphPatch::Type SelectedPatch = EBuiltInRenderGraphPatch::NONE;
	bool EnableGPUParticles = true;
	bool LockBackBuffer = false;
	int LockedWidth = 0;
	int LockedHeight = 0;

	RHI_API void SetRes(BBTestMode::Type t);
	RHI_API static std::string ToString(BBTestMode::Type t);
	RHI_API ERenderDebugOutput::Type GetDebugRenderMode();
	RHI_API void SetDebugRenderMode(ERenderDebugOutput::Type mode);

	void SetRenderScale(float newscale);
	float GetCurrentRenderScale();
	float MaxRenderScale = 3.0f;
	bool RaytracingEnabled()const;
	RayTracingSettings& GetRTSettings();
	ShadowMappingSettings& GetShadowSettings();
	RHI_API DynamicResolutionSettings& GetDynamicResolutionSettings();
	bool AllowMeshInstancing = false;
	void ValidateForAPI(ERenderSystemType system);
	RendererSettings& GetSettingsForRender();
	EVRHMDMode::Type VRHMDMode = EVRHMDMode::Disabled;
	void ValidateSettings();
	bool AllowNativeVRS = true;
	bool InitSceneDataOnAllGPUs = true;
	static const VRXSettings& GetVRXSettings();
	void MaxSupportedCaps(CapabilityData& MaxData);
	bool ShouldRunGPUTests = false;
private:
	RendererSettings RSettings;
	float RenderScale = 1;
	RayTracingSettings RTSettings;
	ShadowMappingSettings ShadowSettings;
	DynamicResolutionSettings DRSSettings;
	ERenderDebugOutput::Type CurrentDebug = ERenderDebugOutput::Off;
	SFRSettings CurrnetSFRSettings;
	VRXSettings VRXSet;


};

//Props Are Set in the Constructor 
struct RenderConstants
{
	int MAX_DYNAMIC_POINT_SHADOWS;
	int MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	int MAX_LIGHTS;
	int DEFAULT_COPYLIST_POOL_SIZE;
	int LIGHTCULLING_TILE_SIZE;
	int MAX_MESH_INSTANCES;
	RenderConstants();
};