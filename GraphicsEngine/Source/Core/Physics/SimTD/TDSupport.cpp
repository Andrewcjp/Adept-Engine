#include "Source/Core/Stdafx.h"
#include "TDSupport.h"
#include "Core/Components/Component.h"
#include "Core/GameObject.h"
#include "Physics/GenericRigidBody.h"
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"


TDSupport::TDSupport()
{}


TDSupport::~TDSupport()
{}

void TDSupport::OnContact(const ContactPair** Contacts, int numContacts)
{

	for (int PairIdx = 0; PairIdx < numContacts; PairIdx++)
	{
		Collider* ColliderA = nullptr;
		Collider* ColliderB = nullptr;
		if (Contacts[PairIdx]->ShapeA->UserData != nullptr)
		{
			ColliderA = (Collider*)Contacts[PairIdx]->ShapeA->UserData;
		}
		if (Contacts[PairIdx]->ShapeA->UserData != nullptr)
		{
			ColliderB = (Collider*)Contacts[PairIdx]->ShapeA->UserData;
		}
		if (ColliderA == nullptr || ColliderB == nullptr)
		{
			Log::LogMessage("untracked Collision ", Log::Severity::Warning);
			continue;
		}
		CollisonData Data;
		//todo: check order here!
		Data.Hitcollider = ColliderA;
		Data.OtherCollider = ColliderB;

		if (ColliderA->GetOwner() != nullptr && ColliderA->GetOwner()->GetOwnerComponent() != nullptr && ColliderA->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderA->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnCollide(Data);
		}
		Data.Hitcollider = ColliderB;
		Data.OtherCollider = ColliderA;
		if (ColliderB->GetOwner() != nullptr && ColliderB->GetOwner()->GetOwnerComponent() != nullptr && ColliderB->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderB->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnCollide(Data);
		}
	}
}

void TDSupport::OnTrigger(const ContactPair** Contacts, int numContacts)
{
	for (int PairIdx = 0; PairIdx < numContacts; PairIdx++)
	{
		Collider* ColliderA = nullptr;
		Collider* ColliderB = nullptr;
		if (Contacts[PairIdx]->ShapeA->UserData != nullptr)
		{
			ColliderA = (Collider*)Contacts[PairIdx]->ShapeA->UserData;
		}
		if (Contacts[PairIdx]->ShapeB->UserData != nullptr)
		{
			ColliderB = (Collider*)Contacts[PairIdx]->ShapeB->UserData;
		}
		if (ColliderA == nullptr || ColliderB == nullptr)
		{
			Log::LogMessage("untracked Trigger Collision ", Log::Severity::Warning);
			continue;
		}
		CollisonData Data;
		//todo: check order here!
		Data.Hitcollider = ColliderA;
		Data.OtherCollider = ColliderB;

		if (ColliderA->GetOwner() != nullptr && ColliderA->GetOwner()->GetOwnerComponent() != nullptr && ColliderA->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderA->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnTrigger(Data);
		}
		Data.Hitcollider = ColliderB;
		Data.OtherCollider = ColliderA;
		if (ColliderB->GetOwner() != nullptr && ColliderB->GetOwner()->GetOwnerComponent() != nullptr && ColliderB->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderB->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnTrigger(Data);
		}
	}
}
