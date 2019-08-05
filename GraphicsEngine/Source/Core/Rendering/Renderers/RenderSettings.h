#pragma once
#include "Core\Platform\ConsoleVariable.h"
#include "..\RenderNodes\RenderGraphSystem.h"

class Archive;
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
namespace BBTestMode
{
	enum Type
	{
		HD,
		QHD,
		UHD,
		Limit
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
//Props Are Set in the Constructor 
struct RenderSettings
{
	RenderSettings();
public:

	AAMode::Type CurrentAAMode = AAMode::FXAA;
	EBuiltinRenderGraphs::Type SelectedGraph = EBuiltinRenderGraphs::DeferredRenderer;
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
	DynamicResolutionSettings& GetDynamicResolutionSettings();
	bool AllowMeshInstancing = false;
	void ValidateForAPI(ERenderSystemType system);
	RendererSettings& GetSettingsForRender();
	EVRHMDMode::Type VRHMDMode = EVRHMDMode::Disabled;
	void ValidateSettings();
	bool AllowNativeVRS = true;
private:
	RendererSettings RSettings;
	float RenderScale = 1;
	RayTracingSettings RTSettings;
	ShadowMappingSettings ShadowSettings;
	DynamicResolutionSettings DRSSettings;
	ERenderDebugOutput::Type CurrentDebug = ERenderDebugOutput::Off;
};
//Props Are Set in the Constructor 
struct MultiGPUMode
{
	MultiGPUMode();
	bool MainPassSFR = false;
	//Splits the Work per Shadow light across the cards
	bool SplitShadowWork = false;
	//Instead of copying the entire map only copies a sampled version for the current frame.
	bool ComputePerFrameShadowDataOnExCard = false;
	//split Particle system compute work across both cards.
	bool PSComputeWorkSplit = false;
	bool AsyncShadows = false;
	int MAX_PRESAMPLED_SHADOWS = 1;
	int MAX_PRESAMPLED_SHADOWS_GPU0 = 4;
	int ShadowLightsOnDev1 = 1;
	bool SFRSplitShadows = false;
	float SecondCardShadowScaleFactor = 1.0f;
	float PreSampleBufferScale = 1.0f;
	bool ShowSplit = false;
	float SFRRatio = 0.5;
	void ValidateSettings();
	//ensure that all GPUs have a scene renderer CBV instance and Transfrom CBV
	//Will affect performance of CBV writes due to duplication.
	bool InitSceneDataOnAllGPUs = true;
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