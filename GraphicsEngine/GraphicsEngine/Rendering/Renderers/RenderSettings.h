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

