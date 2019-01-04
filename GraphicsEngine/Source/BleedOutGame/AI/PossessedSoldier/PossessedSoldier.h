#pragma once
#include "Source/BleedOutGame/AI/B_AIBase.h"
class PossessedSoldier :public B_AIBase
{
public:
	PossessedSoldier();
	~PossessedSoldier();
	void LookAt(glm::vec3 pos);
	GameObject* WeaponBone = nullptr;
	GameObject* VisualMesh = nullptr;
protected:
	virtual void SetupBrain() override;
	AnimationController * CreateAnimationController() override;
	virtual void Update(float dt) override;
};

