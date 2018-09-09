#include "stdafx.h"
#include "RenderSettings.h"
#include "RHI/RHI.h"
#include "Core/Platform/ConsoleVariable.h"
static ConsoleVariable UseDeferredMode("deferred", 0, ECVarType::LaunchOnly);

MultiGPUMode::MultiGPUMode()
{
	MainPassSFR = false;
	SplitShadowWork = false;
	ComputePerFrameShadowDataOnExCard = false;
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
	ShadowMapSize = 1024;
	IsDeferred = UseDeferredMode.GetBoolValue();
	if (IsDeferred)
	{
		Log::OutS << "Starting in Deferred Rendering mode" << Log::OutS;
	}
}
