#include "stdafx.h"
#include "RenderSettings.h"

MultiGPUMode::MultiGPUMode()
{
	MainPassSFR = false;
	SplitShadowWork = true;
	ComputePerFrameShadowDataOnExCard = false;
	PSComputeWorkSplit = false;
}
