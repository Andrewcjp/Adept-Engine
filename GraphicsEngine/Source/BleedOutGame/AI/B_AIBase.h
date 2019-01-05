#pragma once
#include "AI/Core/AIBase.h"

class Weapon;
class BleedOutGameMode;
class B_AIBase : public AIBase
{
public:
	B_AIBase();
	~B_AIBase();
	bool FireAt(glm::vec3 pos);
	void OnDead();
	Weapon* MainWeapon = nullptr;

	virtual void SceneInitComponent() override;

protected:
	BleedOutGameMode* GameMode = nullptr;
private:
	virtual void InitComponent() override;

};

