#pragma once
#include "Source/TestGame/AI/B_AIBase.h"
class PossessedSoldier:public B_AIBase
{
public:
	PossessedSoldier();
	~PossessedSoldier();
	void LookAt(glm::vec3 pos);
	GameObject* WeaponBone = nullptr;
protected:
	virtual void SetupBrain() override;

	AnimationController * CreateAnimationController() override;

	

};

