#pragma once

#include "resource.h"

struct EnginePersistentData
{
	BBTestMode::Type ScreenMode = BBTestMode::Limit;
	MGPUMode::Type MutliGPuMode = MGPUMode::None;
	bool Restart = false;
	int launchCount = 0;
	int BenchIndex = 0;
};