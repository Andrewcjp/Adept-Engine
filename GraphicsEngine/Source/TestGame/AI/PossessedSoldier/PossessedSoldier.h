#pragma once
#include "Source/TestGame/AI/B_AIBase.h"
class PossessedSoldier:public B_AIBase
{
public:
	PossessedSoldier();
	~PossessedSoldier();
protected:
	virtual void SetupBrain() override;

};

