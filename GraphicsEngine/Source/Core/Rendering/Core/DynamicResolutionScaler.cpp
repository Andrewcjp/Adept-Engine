#include "Stdafx.h"
#include "DynamicResolutionScaler.h"
#include "Core\Performance\PerfManager.h"
#include "Core\Utils\StringUtil.h"
//#MGPU: how to handle this?

DynamicResolutionScaler::DynamicResolutionScaler()
{
	Init();
}

DynamicResolutionScaler::~DynamicResolutionScaler()
{}

void DynamicResolutionScaler::Init()
{
	Timer = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Total GPU0"));
	LogEnsure(Timer);
	SetTargetFrameRate(300);
	CurrentCooldown = UpdateRate;
}

void DynamicResolutionScaler::Tick()
{
	if (!RHI::GetRenderSettings()->EnableDynamicResolutionScaling)
	{
		//force update to current render scale
		return;
	}
	CurrentCooldown -= Engine::GetDeltaTime();
	if (CurrentCooldown > 0.0f)
	{
		return;
	}
	CurrentCooldown = UpdateRate;
	float currnetscale = RHI::GetRenderSettings()->GetCurrentRenderScale();
	float CurrnetTime = Timer->Time;
	float delta = TargetGPUTimeMS - CurrnetTime;

	float change = MaxChange * delta;
	change = glm::clamp(change, -MaxChange, MaxChange);
	float newscale = currnetscale + change;
	RHI::GetRenderSettings()->SetRenderScale(newscale);
	Log::LogMessage("Ratio updated to " + StringUtils::ToStringFloat(RHI::GetRenderSettings()->GetCurrentRenderScale()));
}

void DynamicResolutionScaler::SetTargetFrameRate(int rate)
{
	TargetGPUTimeMS = (1.0f / rate)*1000.0f;
}
