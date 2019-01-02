#include "BT_PossessedSoldier.h"
#include "PossessedSoldier.h"
#include "AI/Core/AIController.h"
#include "Source/TestGame/AI/HellKnight/HellKnight_Anim_Controller.h"


PossessedSoldier::PossessedSoldier()
{}

PossessedSoldier::~PossessedSoldier()
{}

void PossessedSoldier::SetupBrain()
{
	BTTree = new BT_PossessedSoldier();
	Controller->Speed = 10.0f;
	AIBase::SetupBrain();
}
AnimationController* PossessedSoldier::CreateAnimationController()
{
	return new HellKnight_Anim_Controller();
}
void PossessedSoldier::LookAt(glm::vec3 pos)
{
	//glm::vec3 DirToTarget = pos - WeaponBone->GetPosition();
	//DirToTarget.z = 0;
	//DirToTarget.x = 0;
	//DirToTarget = glm::normalize(DirToTarget);
	////float angle = glm::degrees(glm::atan(DirToTarget.x, DirToTarget.z));
	////glm::quat rot = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0));
	////WeaponBone->GetTransform()->SetQrot(rot);
	//WeaponBone->GetTransform()->SetQrot(glm::normalize(glm::quatLookAtRH(DirToTarget, glm::vec3(1, 0, 0))));
	
}