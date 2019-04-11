#pragma once
#include "AI/B_AIBase.h"
/*!  \addtogroup Game_AI
* @{ */
class HellKnight : public B_AIBase
{
public:
	HellKnight();
	~HellKnight();
protected:
	virtual void SetupBrain() override;


	virtual AnimationController* CreateAnimationController() override;

};

