
#include "RenderSettings.h"
#include "RHI/RHI.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Assets/Archive.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);

MultiGPUMode::MultiGPUMode()
{
	MainPassSFR = true;
	SplitShadowWork = false;
	ComputePerFrameShadowDataOnExCard = true;
	PSComputeWorkSplit = false;
	AsyncShadows = false; 
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

void MultiGPUMode::Seralise(Archive * A)
{
	ArchiveProp(MainPassSFR);
	ArchiveProp(SplitShadowWork);
	ArchiveProp(PSComputeWorkSplit);
	ArchiveProp(AsyncShadows);
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
