#include "stdafx.h"
#include "RenderSettings.h"

MultiGPUMode::MultiGPUMode()
{
	MainPassSFR = false;
	SplitShadowWork = false;
	ComputePerFrameShadowDataOnExCard = false;
	PSComputeWorkSplit = false;
}
