#pragma once
#include "Source/TestGame/AI/B_AIBase.h"
class HellKnight : public B_AIBase
{
public:
	HellKnight();
	~HellKnight();
protected:
	virtual void SetupBrain() override;

};

