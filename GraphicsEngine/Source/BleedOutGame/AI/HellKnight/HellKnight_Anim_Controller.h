#pragma once
#include "AI\Core\AnimationController.h"
class HellKnight_Anim_Controller : public AnimationController
{
public:
	HellKnight_Anim_Controller();
	~HellKnight_Anim_Controller();

	virtual void OnTick(float dt) override;

};

