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
	UseSFR.SetValue(true);
	//SplitShadows.SetValue(true);
	//AsyncShadow.SetValue(true);
	//SFRSplitShadowsVar.SetValue(true);
	MAX_PRESAMPLED_SHADOWS = 4;
	SecondCardShadowScaleFactor = 1.0f;
	SyncSettings();
	if (Engine::GetEPD()->Restart)
	{
		CurrnetTestMode = Engine::GetEPD()->MutliGPuMode;
	}
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
			break;
		case MGPUMode::ASYNC_SHADOWS:
			AsyncShadows = true;
			SplitShadowWork = true;
			break;
		case MGPUMode::MULTI_SHADOWS:
			SplitShadowWork = true;
			break;
		case MGPUMode::SFR_SHADOWS:
			MainPassSFR = true;
			SFRSplitShadows = true;
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
	IsDeferred = true;
	EnableGPUParticles = false;
	if (IsDeferred)
	{
		Log::OutS << "Starting in Deferred Rendering mode" << Log::OutS;
	}
	RenderScale = 1.0f;
	SetRes(BBTestMode::Limit);

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
		break;
	case MGPUMode::SFR:
		return "SFR";
		break;
	case MGPUMode::SFR_SHADOWS:
		return "SFR_SHADOWS";
		break;
	case MGPUMode::MULTI_SHADOWS:
		return "MULTI_SHADOWS";
		break;
	case MGPUMode::ASYNC_SHADOWS:
		return "ASYNC_SHADOWS";
		break;
	case MGPUMode::Limit:
		break;
	}
	return std::string();
}
