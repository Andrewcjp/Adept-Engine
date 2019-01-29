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
//Props Are Set in the Constructor 
struct RenderSettings
{
	RenderSettings();
public:
	float RenderScale = 1;
	AAMode::Type CurrentAAMode = AAMode::FXAA;
	int ShadowMapSize = 2048;
	bool IsDeferred = false;
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
	void ValidateSettings();
};
//Props Are Set in the Constructor 
struct RenderConstants
{
	int MAX_DYNAMIC_POINT_SHADOWS;
	int MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	int MAX_LIGHTS;
	RenderConstants();
};