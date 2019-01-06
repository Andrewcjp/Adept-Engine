#pragma once
#include "Source/BleedOutGame/AI/B_AIBase.h"
/*!  \addtogroup Game_AI
* @{ */
class PossessedSoldier :public B_AIBase
{
public:
	PossessedSoldier();
	~PossessedSoldier();
	GameObject* WeaponBone = nullptr;
	GameObject* VisualMesh = nullptr;
protected:
	virtual void SetupBrain() override;
	AnimationController * CreateAnimationController() override;
	virtual void Update(float dt) override;
};

