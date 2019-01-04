#include "B_AIBase.h"
#include "Core/GameObject.h"
#include "Core/Utils/MathUtils.h"
#include "Source/BleedOutGame/Components/Health.h"
#include "Source/BleedOutGame/Components/Weapon.h"
#include "Source/BleedOutGame/BleedOutPCH.h"


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

void B_AIBase::OnDead()
{
	AIBase::SetDead();
}

void B_AIBase::InitComponent()
{
	AIBase::InitComponent();
	MainWeapon = GetOwner()->GetComponent<Weapon>();
	Health* H = GetOwner()->GetComponent<Health>();
	H->BindDeathCallback(std::bind(&B_AIBase::OnDead, this));
}
