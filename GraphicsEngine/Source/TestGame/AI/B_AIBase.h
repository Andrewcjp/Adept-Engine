#pragma once
#include "AI/Core/AIBase.h"

class Weapon;
class B_AIBase : public AIBase
{
public:
	B_AIBase();
	~B_AIBase();
	bool FireAt(glm::vec3 pos);

	Weapon* MainWeapon = nullptr;
private:


	virtual void InitComponent() override;

};

