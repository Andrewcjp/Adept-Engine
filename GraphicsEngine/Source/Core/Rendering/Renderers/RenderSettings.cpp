
#include "RenderSettings.h"
#include "RHI/RHI.h"
#include "Core/Platform/ConsoleVariable.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);

MultiGPUMode::MultiGPUMode()
{
	MainPassSFR = false;
	SplitShadowWork = true;
	ComputePerFrameShadowDataOnExCard = true;
	PSComputeWorkSplit = false;
}

void MultiGPUMode::ValidateSettings()
{
	if (!RHI::UseAdditionalGPUs() || RHI::GetDeviceCount() == 1)
	{
		MainPassSFR = false;
		SplitShadowWork = false;
		ComputePerFrameShadowDataOnExCard = false;
		PSComputeWorkSplit = false;
	}
}

RenderSettings::RenderSettings()
{
	ShadowMapSize = 2048;
	IsDeferred = UseDeferredMode.GetBoolValue();
	IsDeferred = true;
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
