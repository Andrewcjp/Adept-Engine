#pragma once
enum AAMode
{
	NONE,
	FXAA,
	MSAA,
	SMAA,
};

struct RenderSettings
{
public:
	float RenderScale = 1;
	AAMode CurrentAAMode = AAMode::FXAA;
};

struct MultiGPUMode
{
	MultiGPUMode();
	bool MainPassSFR = false;
	//Splits the Work per Shadow light across the cards
	bool SplitShadowWork = false;
	//Instead of copying the entire map only copies a sampled version for the current frame.
	bool ComputePerFrameShadowDataOnExCard = false;
	//split Particle system compute work across both cards.
	bool PSComputeWorkSplit = false;

};
