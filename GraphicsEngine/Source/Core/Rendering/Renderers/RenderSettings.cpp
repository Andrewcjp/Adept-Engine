#include "RenderSettings.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);
static ConsoleVariable UseSFR("UseSFR", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitShadows("SplitShadows", false, ECVarType::LaunchOnly);
static ConsoleVariable AsyncShadow("AsyncShadow", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitPS("SplitPS", false, ECVarType::LaunchOnly);
static ConsoleVariable PreComputePerFrameShadowData("ComputePerFrameShadowDataOnExCard", true, ECVarType::LaunchOnly);

MultiGPUMode::MultiGPUMode()
{
	//UseSFR.SetValue(true);
	//SplitShadows.SetValue(true);
	SyncSettings();
}

std::string BoolToString(bool value)
{
	return value ? "enabled" : "disabled";
}

void MultiGPUMode::SyncSettings()
{
	MainPassSFR = UseSFR.GetBoolValue();
	SplitShadowWork = SplitShadows.GetBoolValue();
	AsyncShadows = AsyncShadow.GetBoolValue();
	PSComputeWorkSplit = SplitPS.GetBoolValue();
	ComputePerFrameShadowDataOnExCard = PreComputePerFrameShadowData.GetBoolValue();		
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
	Log::LogMessage("SFR " + BoolToString(MainPassSFR));
	Log::LogMessage("Split shadows " + BoolToString(SplitShadowWork));
	Log::LogMessage("AsyncShadows " + BoolToString(AsyncShadows));
	Log::LogMessage("Particle System MultiGPU " + BoolToString(PSComputeWorkSplit));
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
}
