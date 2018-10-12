#pragma once
#include "RHI/RHI_inc_fwd.h"
#include "Core/EngineTypes.h"
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
private:
	IntPoint Dimentions = IntPoint(1024, 1024);
};

