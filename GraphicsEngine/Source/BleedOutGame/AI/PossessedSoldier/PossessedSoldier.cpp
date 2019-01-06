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
	B_AIBase::Update(dt);
	if (VisualMesh != nullptr)
	{
		/*	glm::vec3 pos = VisualMesh->GetMeshRenderer()->GetPosOfBone("mixamorig:LeftHand");
			DebugDrawers::DrawDebugSphere(pos, 0.5f, glm::vec3(1));*/
	}
}

