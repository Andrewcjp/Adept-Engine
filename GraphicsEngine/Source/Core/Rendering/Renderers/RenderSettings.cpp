#include "RenderSettings.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);
static ConsoleVariable UseSFR("UseSFR", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitShadows("SplitShadows", false, ECVarType::LaunchOnly);
static ConsoleVariable AsyncShadow("AsyncShadow", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitPS("SplitPS", false, ECVarType::LaunchOnly);
static ConsoleVariable PreComputePerFrameShadowData("ComputePerFrameShadowDataOnExCard", true, ECVarType::LaunchOnly);
static ConsoleVariable SFRSplitShadowsVar("SFRSplitShadows", false, ECVarType::LaunchOnly);
MultiGPUMode::MultiGPUMode()
{
	UseSFR.SetValue(true);
	//SplitShadows.SetValue(true);
	//AsyncShadow.SetValue(true);
	SFRSplitShadowsVar.SetValue(true);
	MAX_PRESAMPLED_SHADOWS = 4;
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
	if (!SplitShadowWork)
	{
		AsyncShadows = false;
	}
	if (MainPassSFR)
	{
		AsyncShadows = false;
		SplitShadowWork = false;
	}
	else
	{
		SFRSplitShadows = false;
	}
	int Offset = 30;
	Log::LogBoolTerm("SFR ", MainPassSFR, Offset);
	Log::LogBoolTerm("Split SFR shadows ", SFRSplitShadows, Offset);
	Log::LogBoolTerm("Split shadows ", SplitShadowWork, Offset);
	Log::LogBoolTerm("Async Shadows ", AsyncShadows, Offset);
	//Log::LogBoolTerm("Particle System MultiGPU ", PSComputeWorkSplit, Offset);
}

bool MultiGPUMode::UseSplitShadows() const
{
	return SplitShadowWork || SFRSplitShadows;
}

RenderSettings::RenderSettings()
{
	ShadowMapSize = 1024;
	IsDeferred = UseDeferredMode.GetBoolValue();
	IsDeferred = true;
	EnableGPUParticles = false;
	if (IsDeferred)
	{
		Log::OutS << "Starting in Deferred Rendering mode" << Log::OutS;
	}
	RenderScale = 1.0f;
	LockBackBuffer = true;
	if (LockBackBuffer)
	{
		SetRes(BBTestMode::HD);
	}
}

void RenderSettings::SetRes(BBTestMode::Type Mode)
{
	switch (Mode)
	{
	case BBTestMode::HD:
		LockedWidth = 1920;
		LockedHeight = 1080;
		break;
	case BBTestMode::QHD:
		LockedWidth = 2560;
		LockedHeight = 1440;
		break;
	case BBTestMode::UHD:
		LockedWidth = 3840;
		LockedHeight = 2160;
		break;
	}
}

RenderConstants::RenderConstants()
{
	MAX_DYNAMIC_POINT_SHADOWS = 4;
	MAX_DYNAMIC_DIRECTIONAL_SHADOWS = 1;
	MAX_LIGHTS = 8;
	DEFAULT_COPYLIST_POOL_SIZE = 4;
}

std::string RenderSettings::ToString(BBTestMode::Type t)
{
	switch (t)
	{
	case BBTestMode::HD:
		return "HD 1080P";
		break;
	case BBTestMode::QHD:
		return "QHD 1440P";
		break;
	case BBTestMode::UHD:
		return "UHD 2160P";
		break;
	}
	return "?";
}
