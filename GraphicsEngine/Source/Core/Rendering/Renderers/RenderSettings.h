#pragma once
#include "Core\Platform\ConsoleVariable.h"

class Archive;
namespace AAMode
{
	enum Type
	{
		NONE,
		FXAA,
		MSAA,
		SMAA,
	};
}
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
//Props Are Set in the Constructor 
struct RenderSettings
{
	RenderSettings();
public:
	float RenderScale = 1;
	AAMode::Type CurrentAAMode = AAMode::FXAA;
	int ShadowMapSize = 2048;
	bool IsDeferred = false;
	bool EnableGPUParticles = true;
	bool LockBackBuffer = false;
	int LockedWidth = 0;
	int LockedHeight = 0;
	RHI_API void SetRes(BBTestMode::Type t);
	RHI_API static std::string ToString(BBTestMode::Type t);
};
//Props Are Set in the Constructor 
struct MultiGPUMode
{
	MultiGPUMode();
	void SyncSettings();
	bool MainPassSFR = false;
	//Splits the Work per Shadow light across the cards
	bool SplitShadowWork = false;
	//Instead of copying the entire map only copies a sampled version for the current frame.
	bool ComputePerFrameShadowDataOnExCard = false;
	//split Particle system compute work across both cards.
	bool PSComputeWorkSplit = false;
	bool AsyncShadows = false;
	int MAX_PRESAMPLED_SHADOWS = 1;
	bool SFRSplitShadows = false;
	float SecondCardShadowScaleFactor = 1.0f;
	void ValidateSettings();
};
//Props Are Set in the Constructor 
struct RenderConstants
{
	int MAX_DYNAMIC_POINT_SHADOWS;
	int MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	int MAX_LIGHTS;
	int DEFAULT_COPYLIST_POOL_SIZE;
	RenderConstants();
};