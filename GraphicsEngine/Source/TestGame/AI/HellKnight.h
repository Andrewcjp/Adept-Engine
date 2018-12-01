#pragma once
#include "AI/Core/AIBase.h"
class HellKnight : public AIBase
{
public:
	HellKnight();
	~HellKnight();
protected:
	virtual void SetupBrain() override;

};

