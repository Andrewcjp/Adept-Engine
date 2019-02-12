#include "RenderSettings.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);
static ConsoleVariable UseSFR("UseSFR", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitShadows("SplitShadows", false, ECVarType::LaunchOnly);
static ConsoleVariable AsyncShadow("AsyncShadow", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitPS("SplitPS", false, ECVarType::LaunchOnly);
static ConsoleVariable PreComputePerFrameShadowData("ComputePerFrameShadowDataOnExCard", true, ECVarType::LaunchOnly);
static ConsoleVariable SFRSplitShadowsVar("SFRSplitShadows", true, ECVarType::LaunchOnly);
MultiGPUMode::MultiGPUMode()
{
	//UseSFR.SetValue(true);
	SplitShadows.SetValue(true);
	MAX_PRESAMPLED_SHADOWS = 2;
	SecondCardShadowScaleFactor = 1.0f;
	SyncSettings();
}

void MultiGPUMode::SyncSettings()
{
	MainPassSFR = UseSFR.GetBoolValue();
	SplitShadowWork = SplitShadows.GetBoolValue();
	AsyncShadows = AsyncShadow.GetBoolValue();
	PSComputeWorkSplit = SplitPS.GetBoolValue();
	ComputePerFrameShadowDataOnExCard = PreComputePerFrameShadowData.GetBoolValue();
	SFRSplitShadows = SFRSplitShadowsVar.GetBoolValue();
}

void MultiGPUMode::ValidateSettings()
{
	if (!RHI::UseAdditionalGPUs() || RHI::GetDeviceCount() == 1 || !RHI::IsD3D12())
	{
		MainPassSFR = false;
		SplitShadowWork = false;
		ComputePerFrameShadowDataOnExCard = false;
		PSComputeWorkSplit = false;
		AsyncShadows = false;
	}
	int Offset = 30;
	Log::LogBoolTerm("SFR ", MainPassSFR, Offset);
	Log::LogBoolTerm("Split shadows ", SplitShadowWork, Offset);
	Log::LogBoolTerm("Split SFR shadows ", SFRSplitShadows, Offset);
	Log::LogBoolTerm("Async Shadows ", AsyncShadows, Offset);
	Log::LogBoolTerm("Particle System MultiGPU ", PSComputeWorkSplit, Offset);
}

RenderSettings::RenderSettings()
{
	ShadowMapSize = 1024;
	IsDeferred = UseDeferredMode.GetBoolValue();
	IsDeferred = false;
	if (IsDeferred)
	{
		Log::OutS << "Starting in Deferred Rendering mode" << Log::OutS;
	}
	RenderScale = 1.0f;
}

RenderConstants::RenderConstants()
{
	MAX_DYNAMIC_POINT_SHADOWS = 4;
	MAX_DYNAMIC_DIRECTIONAL_SHADOWS = 1;
	MAX_LIGHTS = 8;
	DEFAULT_COPYLIST_POOL_SIZE = 4;
}
