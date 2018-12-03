#include "Source/TestGame/TestGamePCH.h"
#include "B_AIBase.h"
#include "../Components/Weapon.h"
#include "Core/Utils/MathUtils.h"


B_AIBase::B_AIBase()
{}


B_AIBase::~B_AIBase()
{}

bool B_AIBase::FireAt(glm::vec3 pos)
{
	//detect angle delta 
	//fire x shots
	if (MainWeapon->GetOwner() != GetOwner())
	{
		glm::vec3 direction = MainWeapon->GetOwner()->GetPosition() - pos;
		direction = MathUtils::SafeNormalise(direction);
		//MainWeapon->GetOwner()->GetTransform()->SetLocalRotation(glm::quat(0, 1, 0, 0));
	}
	return MainWeapon->Fire();
}

 void B_AIBase::InitComponent()
{
	 AIBase::InitComponent();
	 MainWeapon = GetOwner()->GetComponent<Weapon>();
}
