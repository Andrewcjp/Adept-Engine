#include "BT_PossessedSoldier.h"
#include "PossessedSoldier.h"
#include "AI/Core/AIController.h"
#include "Source/BleedOutGame/AI/HellKnight/HellKnight_Anim_Controller.h"
#include "Core/GameObject.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Utils/DebugDrawers.h"


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

void PossessedSoldier::Update(float dt)
{
	AIBase::Update(dt);
	if (VisualMesh != nullptr)
	{
		glm::vec3 pos = VisualMesh->GetMeshRenderer()->GetPosOfBone("mixamorig:LeftHand");
		//DebugDrawers::DrawDebugSphere(pos, 0.5f, glm::vec3(1,0,0));
		//glm::mat4x4 Model = (WeaponBone->GetTransform()->GetModel());
		//pos = glm::vec4(pos, 1.0f) * (Model);
		//pos += glm::vec3(-0.5f,0.65,0);
		DebugDrawers::DrawDebugSphere(pos, 0.5f, glm::vec3(1));
		//WeaponBone->GetTransform()->SetLocalPosition(pos);
		//WeaponBone->SetPosition(pos);
		//WeaponBone->GetTransform()->SetLocalPosition();
	}
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