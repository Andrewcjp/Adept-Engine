#include "SkullChaser.h"
#include "Core/GameObject.h"
#include "Core/Components/Core_Components_inc.h"

SkullChaser::SkullChaser()
{}


SkullChaser::~SkullChaser()
{}

void SkullChaser::InitComponent()
{

}

void SkullChaser::Update(float dt)
{
	AIBase::Update(dt);
	if (DistanceToPlayer < 50 && Player != nullptr)
	{
		glm::mat4 rotat = glm::lookAtLH(GetOwner()->GetPosition(), Player->GetPosition(), glm::vec3(0, -1, 0));
		GetOwner()->SetRotation(rotat);
		GetOwner()->GetComponent<RigidbodyComponent>()->SetLinearVelocity(GetOwner()->GetTransform()->GetForward() * 10);
	}
}
