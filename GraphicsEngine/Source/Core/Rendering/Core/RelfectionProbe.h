#pragma once
#include "RHI/RHI_inc_fwd.h"
#include "Core/EngineTypes.h"
namespace EReflectionProbeMode
{
	enum Type
	{
		ERealTime,
		EOneSidePreframe,
		Baked,
		Limit
	};
}
class RelfectionProbe
{
public:
	RelfectionProbe();
	~RelfectionProbe();
	FrameBuffer* CapturedTexture = nullptr;
	IntPoint GetDimentions()const
	{
		return Dimentions;
	}
	EReflectionProbeMode::Type ProbeMode = EReflectionProbeMode::Baked;
	bool NeedsCapture() const;
	void SetCaptured();
private:
	IntPoint Dimentions = IntPoint(1024, 1024);
	bool IsCaptured = false;
};

