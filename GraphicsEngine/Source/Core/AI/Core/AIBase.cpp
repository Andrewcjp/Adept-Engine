#include "Stdafx.h"
#include "AIBase.h"
#include "Core/GameObject.h"

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

}
