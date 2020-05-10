#pragma once
#include "Core\Platform\ConsoleVariable.h"
#include "..\RenderNodes\RenderGraphSystem.h"
#include "WinLauncher.h"
#include "Core\Reflection\IReflect.h"

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
		Scene_Voxel,
		RelfectionBuffer,
		Scene_EdgeDetect,
		Scene_EdgeDetectCount,
		Scene_Metallic,
		Scene_RoughNess,
		Scene_LightRange,
		Scene_UVs,
		Scene_ShaderInstructionCount,
		Limit
	};
};

struct RayTracingSettings
{
	bool Enabled = false;
};
struct VoxelSettings
{
	bool Enabled = false;
	bool UseConeTracing = false;
	bool UseDXR = false;
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
	bool Enabled = false;
	//adds gaps in Render targets to show transitions
	bool DEBUG_ShowSplits = false;
	//each pixel will have a colour tint which indicates to the GPU that rendered it.
	bool DEBUG_ColourPixelsPerGPU = false;
	bool Use8BitCompression = false;
};

AENUM();
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
	bool EnableVRX = false;
	bool VRXActive = true;
	EVRSMode::Type VRXMode = EVRSMode::HardwareAndSoftware;
	bool SupportPerDrawRateOnVRR = false;	//not yet supported
	int VRRTileSize = 16;
	bool EnableVarableRateSuperSampling = false;
	bool UseVRX(DeviceContext* con = nullptr)const;
	bool UseVRR(DeviceContext* con = nullptr)const;
	bool UseVRS(DeviceContext* con = nullptr)const;
};
//Props Are Set in the Constructor 
UCLASS();
struct RenderSettings : public IReflect
{
	RenderSettings();
	CLASS_BODY_Reflect();
public:
	PROPERTY(Name = "AA Mode");
	AAMode::Type CurrentAAMode = AAMode::NONE;
	EBuiltinRenderGraphs::Type SelectedGraph = EBuiltinRenderGraphs::DeferredRenderer;
	EBuiltInRenderGraphPatch::Type SelectedPatch = EBuiltInRenderGraphPatch::NONE;
	
	bool EnableGPUParticles = true;
	bool LockBackBuffer = false;
	BBTestMode::Type Testmode = BBTestMode::Limit;
	int LockedWidth = 0;
	int LockedHeight = 0;

	RHI_API void SetRes(BBTestMode::Type t);
	RHI_API static std::string ToString(BBTestMode::Type t);
	RHI_API ERenderDebugOutput::Type GetDebugRenderMode();
	RHI_API void SetDebugRenderMode(ERenderDebugOutput::Type mode);

	void SetRenderScale(float newscale);
	float GetCurrentRenderScale();
	PROPERTY(Name = "Max Render Scale");
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
	bool InitSceneDataOnAllGPUs = true;
	RHI_API static const VRXSettings& GetVRXSettings();
	static void SetVRXActive(bool state);
	void MaxSupportedCaps(CapabilityData& MaxData);
	bool ShouldRunGPUTests = false;
	VoxelSettings& GetVoxelSet();
	SFRSettings& GetCurrnetSFRSettings() { return CurrnetSFRSettings; }
	bool RequestAllGPUs = false;
private:
	PROPERTY(Name = "Render Scale");
	float RenderScale = 1;
	RendererSettings RSettings;
	RayTracingSettings RTSettings;
	ShadowMappingSettings ShadowSettings;
	DynamicResolutionSettings DRSSettings;
	ERenderDebugOutput::Type CurrentDebug = ERenderDebugOutput::Off;
	SFRSettings CurrnetSFRSettings;
	VRXSettings VRXSet;
	VoxelSettings VoxelSet;


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