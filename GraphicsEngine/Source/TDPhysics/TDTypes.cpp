#include "TDTypes.h"
#include "TDShape.h"
#include "Utils/MathUtils.h"

TD::TDPhysicalMaterial::TDPhysicalMaterial()
{
	Restitution = 0.2f;
}

void TD::ContactData::Contact(glm::vec3 position, glm::vec3 normal, float seperation)
{
	Blocking = true;
	ContactPoints[ContactCount] = position;
	if (normal != glm::vec3(0))
	{
		Direction[ContactCount] = glm::normalize(normal);
	}
	else
	{
		Direction[ContactCount] = glm::vec3(0, 1, 0);
	}
	depth[ContactCount] = fabsf(seperation);
	CheckNAN(normal);
	CheckNAN(position);
	ContactCount++;
}

void TD::ContactData::Reset()
{
	ContactPoints[MAX_CONTACT_POINTS_COUNT];
	Blocking = false;
	Direction[MAX_CONTACT_POINTS_COUNT];
	depth[MAX_CONTACT_POINTS_COUNT];
	ContactCount = 0;
}

void TD::TDFlagsBase::SetFlagValue(int flag, bool value)
{
	if (value)
	{
		Flags |= flag;
	}
	else
	{
		Flags &= ~flag;
	}
}

bool TD::TDFlagsBase::GetFlagValue(int flag)
{
	return Flags & flag;
}

void TD::TDFlagsBase::SetFlags(int flags)
{
	Flags = Flags;
}

TD::CollisionPair::CollisionPair(TDActor * A, TDActor * B)
{
	first = A;
	second = B;
	IsTriggerPair = first->GetAttachedShapes()[0]->GetFlags().GetFlagValue(TDShapeFlags::ETrigger) || second->GetAttachedShapes()[0]->GetFlags().GetFlagValue(TDShapeFlags::ETrigger);
}

void TD::RaycastData::AddContact(glm::vec3 point, glm::vec3 normal, float depth)
{
	BlockingHit = true;
	Points[Count].Distance = depth;
	Points[Count].Point = point;
	Points[Count].Normal = normal;
	Count++;
	DebugEnsure(Count == 1);
}

void TD::RaycastData::Reset()
{
	Points[MAX_CONTACT_POINTS_COUNT];
	BlockingHit = false;
	Count = 0;
}
