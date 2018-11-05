#pragma once
#include "AI/Core/AIBase.h"
class DemonImp : public AIBase
{
public:
	DemonImp();
	~DemonImp();
protected:
	virtual void SetupBrain() override;

};

