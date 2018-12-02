#include "Source/TestGame/TestGamePCH.h"
#include "B_AIBase.h"
#include "../Components/Weapon.h"


B_AIBase::B_AIBase()
{}


B_AIBase::~B_AIBase()
{}

bool B_AIBase::FireAt(glm::vec3 pos, int rounds)
{
	//detect angle delta 
	//fire x shots
	return MainWeapon->Fire();
}

 void B_AIBase::InitComponent()
{
	 AIBase::InitComponent();
	 MainWeapon = GetOwner()->GetComponent<Weapon>();
}
