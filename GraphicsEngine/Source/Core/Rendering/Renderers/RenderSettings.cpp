
#include "RenderSettings.h"
#include "RHI/RHI.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Assets/Archive.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);
static ConsoleVariable UseSFR("UseSFR", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitShadows("SplitShadows", false, ECVarType::LaunchOnly);
static ConsoleVariable AsyncShadow("AsyncShadow", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitPS("SplitPS", false, ECVarType::LaunchOnly);
static ConsoleVariable PreComputePerFrameShadowData("ComputePerFrameShadowDataOnExCard", false, ECVarType::LaunchOnly);

MultiGPUMode::MultiGPUMode()
{
	//UseSFR.SetValue(true);
	SyncSettings();
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
	if (!RHI::UseAdditionalGPUs() || RHI::GetDeviceCount() == 1)
	{
		MainPassSFR = false;
		SplitShadowWork = false;
		ComputePerFrameShadowDataOnExCard = false;
		PSComputeWorkSplit = false;
		AsyncShadows = false;
	}
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
