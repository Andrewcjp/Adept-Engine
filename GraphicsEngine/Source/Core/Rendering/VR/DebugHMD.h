#pragma once
#include "HMD.h"
//used to emulate a HMD does not output as that is handled by mirroring 
class DebugHMD : public HMD
{
public:
	DebugHMD();
	virtual ~DebugHMD();
	float roty = 0.0f;
	virtual void Update() override;

};

