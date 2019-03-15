#include "RenderSettings.h"
#include "Core\Engine.h"
#include "GraphicsEngine.h"
static ConsoleVariable UseDeferredMode("deferred", false, ECVarType::LaunchOnly);
static ConsoleVariable UseSFR("UseSFR", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitShadows("SplitShadows", false, ECVarType::LaunchOnly);
static ConsoleVariable AsyncShadow("AsyncShadow", false, ECVarType::LaunchOnly);
static ConsoleVariable SplitPS("SplitPS", false, ECVarType::LaunchOnly);
static ConsoleVariable PreComputePerFrameShadowData("ComputePerFrameShadowDataOnExCard", true, ECVarType::LaunchOnly);
static ConsoleVariable SFRSplitShadowsVar("SFRSplitShadows", false, ECVarType::LaunchOnly);
MultiGPUMode::MultiGPUMode()
{
	MAX_PRESAMPLED_SHADOWS = 4;
	SecondCardShadowScaleFactor = 1.0f;
	SyncSettings();
	if (Engine::GetEPD()->Restart)
	{
		CurrnetTestMode = Engine::GetEPD()->MultiGPUMode;
	}
	else
	{
		CurrnetTestMode = MGPUMode::ASYNC_SHADOWS_1;
	}
	PreSampleBufferScale = 1.0f;
	Log::LogMessage("There are " + std::to_string(MGPUMode::Limit) + " Test cases");
	Log::LogMessage("Starting In Mode: " + MGPUMode::ToString(CurrnetTestMode) + "( " + std::to_string((int)CurrnetTestMode) + " / " + std::to_string((int)MGPUMode::Limit) + ")");
	//ShowSplit = true;
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
	if (CurrnetTestMode != MGPUMode::Limit)
	{
		MainPassSFR = false;
		SplitShadowWork = false;
		ComputePerFrameShadowDataOnExCard = false;
		PSComputeWorkSplit = false;
		AsyncShadows = false;
		switch (CurrnetTestMode)
		{
		case MGPUMode::SFR:
			MainPassSFR = true;
			SFRRatio = 0.5f;
			break;
		case MGPUMode::SFR_RATIOOPTIMIZED:
			MainPassSFR = true;
			SFRRatio = 0.9f;
			break;
		case MGPUMode::ASYNC_SHADOWS_1:
			AsyncShadows = true;
			SplitShadowWork = true;
			MAX_PRESAMPLED_SHADOWS = 1;
			break;
		case MGPUMode::ASYNC_SHADOWS_2:
			AsyncShadows = true;
			SplitShadowWork = true;
			MAX_PRESAMPLED_SHADOWS = 2;
			break;
		case MGPUMode::ASYNC_SHADOWS_3:
			AsyncShadows = true;
			SplitShadowWork = true;
			MAX_PRESAMPLED_SHADOWS = 3;
			break;
		case MGPUMode::ASYNC_SHADOWS_4:
			MAX_PRESAMPLED_SHADOWS = 4;
			AsyncShadows = true;
			SplitShadowWork = true;
			break;
		case MGPUMode::MULTI_SHADOWS_1:
			MAX_PRESAMPLED_SHADOWS = 1;
			SplitShadowWork = true;
			break;
		case MGPUMode::MULTI_SHADOWS_2:
			MAX_PRESAMPLED_SHADOWS = 2;
			SplitShadowWork = true;
			break;
		case MGPUMode::MULTI_SHADOWS_3:
			MAX_PRESAMPLED_SHADOWS = 3;
			SplitShadowWork = true;
			break;
		case MGPUMode::MULTI_SHADOWS_4:
			MAX_PRESAMPLED_SHADOWS = 4;
			SplitShadowWork = true;
			break;
		case MGPUMode::SFR_SHADOWS_0:
			ShadowLightsOnDev1 = 0;
			MainPassSFR = true;
			SFRSplitShadows = true;
			break;
		case MGPUMode::SFR_SHADOWS_1:
			ShadowLightsOnDev1 = 1;
			MainPassSFR = true;
			SFRSplitShadows = true;
			break;
		case MGPUMode::SFR_SHADOWS_2:
			ShadowLightsOnDev1 = 2;
			MainPassSFR = true;
			SFRSplitShadows = true;
			break;
		case MGPUMode::SFR_SHADOWS_3://6 mins
			ShadowLightsOnDev1 = 3;
			MainPassSFR = true;
			SFRSplitShadows = true;
			break;
		case MGPUMode::SFR_SHADOWS_4:
			ShadowLightsOnDev1 = 4;
			MainPassSFR = true;
			SFRSplitShadows = true;
			break;
		case MGPUMode::SFR_SHADOWS_RATIOOPTIMIZED:
			MainPassSFR = true;
			SFRSplitShadows = true;
#if 1
			ShadowLightsOnDev1 = 1;
			SFRRatio = 0.9f;
#else
			ShadowLightsOnDev1 = 1;
			SFRRatio = 0.6f;
#endif
			break;
		}
	}

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
	IsDeferred = false;
	EnableGPUParticles = false;
	if (IsDeferred)
	{
		Log::OutS << "Starting in Deferred Rendering mode" << Log::OutS;
	}
	RenderScale = 1.0f;
	SetRes(BBTestMode::HD);

}

void RenderSettings::SetRes(BBTestMode::Type Mode)
{
	LockBackBuffer = true;
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
	case BBTestMode::Limit:
		LockBackBuffer = false;
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

std::string MGPUMode::ToString(MGPUMode::Type t)
{
	switch (t)
	{
	case MGPUMode::None:
		return "NONE";
	case MGPUMode::SFR:
		return "SFR";
	case MGPUMode::SFR_RATIOOPTIMIZED:
		return "SFR_RATIOOPTIMIZED";
	case MGPUMode::SFR_SHADOWS_0:
		return "SFR_SHADOWS_0";
	case MGPUMode::SFR_SHADOWS_1:
		return "SFR_SHADOWS_1";
	case MGPUMode::SFR_SHADOWS_2:
		return "SFR_SHADOWS_2";
	case MGPUMode::SFR_SHADOWS_3:
		return "SFR_SHADOWS_3";
	case MGPUMode::SFR_SHADOWS_4:
		return "SFR_SHADOWS_4";
	case MGPUMode::SFR_SHADOWS_RATIOOPTIMIZED:
		return "SFR_SHADOWS_RATIOOPTIMIZED";
	case MGPUMode::MULTI_SHADOWS_1:
		return "MULTI_SHADOWS_1";
	case MGPUMode::MULTI_SHADOWS_2:
		return "MULTI_SHADOWS_2";
	case MGPUMode::MULTI_SHADOWS_3:
		return "MULTI_SHADOWS_3";
	case MGPUMode::MULTI_SHADOWS_4:
		return "MULTI_SHADOWS_4";
	case MGPUMode::ASYNC_SHADOWS_1:
		return "ASYNC_SHADOWS_1";
	case MGPUMode::ASYNC_SHADOWS_2:
		return "ASYNC_SHADOWS_2";
	case MGPUMode::ASYNC_SHADOWS_3:
		return "ASYNC_SHADOWS_3";
	case MGPUMode::ASYNC_SHADOWS_4:
		return "ASYNC_SHADOWS_4";
	case MGPUMode::Limit:
		return "LIMIT";
	default:
		break;
	}
	return "ERROR";
}
