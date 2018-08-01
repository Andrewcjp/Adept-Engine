#include "stdafx.h"
#include "RenderSettings.h"
#include "RHI/RHI.h"
MultiGPUMode::MultiGPUMode()
{
	MainPassSFR = false;
	SplitShadowWork = true;
	ComputePerFrameShadowDataOnExCard = false;
	PSComputeWorkSplit = false;

	ValidateSettings();
}

void MultiGPUMode::ValidateSettings()
{
	if (!RHI::UseAdditionalGPUs())
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
}
