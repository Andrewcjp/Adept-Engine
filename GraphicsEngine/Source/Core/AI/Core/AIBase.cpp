#include "Stdafx.h"
#include "AIBase.h"
#include "Core/GameObject.h"
#include "AI/Core/AIController.h"

AIBase::AIBase()
{}


AIBase::~AIBase()
{}

void AIBase::Update(float dt)
{
	if (Player != nullptr)
	{
		DistanceToPlayer = glm::distance(Player->GetPosition(), GetOwner()->GetPosition());
	}
}

void AIBase::InitComponent()
{
	Controller = GetOwner()->GetComponent<AIController>();
}
