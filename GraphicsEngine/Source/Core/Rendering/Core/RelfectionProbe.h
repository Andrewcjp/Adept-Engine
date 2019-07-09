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
	FrameBuffer* ConvolutionBuffer = nullptr;
	IntPoint GetDimentions()const;
	EReflectionProbeMode::Type ProbeMode = EReflectionProbeMode::Baked;
	bool NeedsCapture() const;
	void SetCaptured();
	bool GetNeedsDownSample() const;
	void SetNeedsDownSample(bool val);
private:
	IntPoint Dimentions = IntPoint(1024, 1024);
	int ConvolutionRes = 128;
	bool IsCaptured = false;
	bool NeedsDownSample = false;

};

