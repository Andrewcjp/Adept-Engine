#pragma once

#include "resource.h"
namespace BBTestMode
{
	enum Type
	{
		HD,
		QHD,
		UHD,
		Limit
	};
}
struct EnginePersistentData
{
	BBTestMode::Type ScreenMode = BBTestMode::Limit;
	bool Restart = false;
	int launchCount = 0;
	int BenchIndex = 0;
	float BenchTime = 0.0f;
};
