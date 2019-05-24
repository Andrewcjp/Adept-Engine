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
	UpdateRate = 0.3f;
	DeadZone = 0.3f;
	MaxChange = 0.5f;
	Timer = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Total GPU0"));
	LogEnsure(Timer);
	if (RHI::IsRenderingVR())
	{
		SetTargetFrameRate(95);
	}
	else
	{
		SetTargetFrameRate(300);
	}
	MaxResolutionScale = 1.0f;
	CurrentCooldown = UpdateRate;
}

void DynamicResolutionScaler::Tick()
{
	SCOPE_CYCLE_COUNTER_GROUP("DynamicResolutionScaler", "RHI");
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
	if (abs(delta) < DeadZone)
	{
		return;
	}
	float change = MaxChange * glm::sign(delta) * (abs(delta) / TargetGPUTimeMS);
	change = glm::clamp(change, -MaxChange, MaxChange);
	float newscale = currnetscale + change;
	//newscale = glm::clamp(newscale, 0.1f, MaxResolutionScale);
	RHI::GetRenderSettings()->SetRenderScale(newscale);
	Log::LogMessage("Ratio updated to " + StringUtils::ToStringFloat(RHI::GetRenderSettings()->GetCurrentRenderScale()));
}

void DynamicResolutionScaler::SetTargetFrameRate(int rate)
{
	TargetGPUTimeMS = (1.0f / rate)*1000.0f;
	TargetGPUTimeMS -= DeadZone;
}


