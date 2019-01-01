#include "TDTypes.h"
#include "TDShape.h"
#include "Utils/MathUtils.h"
#include "TDQuerryFilter.h"
#include <algorithm>

TD::TDPhysicalMaterial::TDPhysicalMaterial()
{
	Restitution = 0.5f;
}

void TD::ContactData::Contact(glm::vec3 position, glm::vec3 normal, float seperation)
{
	Blocking = true;
	if (ContactCount >= MAX_CONTACT_POINTS_COUNT)
	{
		return;
	}
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

TD::ActorCollisionPair::ActorCollisionPair(TDActor * A, TDActor * B)
{
	first = A;
	second = B;
	CreateShapePairs();
}

void TD::ActorCollisionPair::CreateShapePairs()
{
	for (int i = 0; i < first->GetAttachedShapes().size(); i++)
	{
		for (int j = 0; j < second->GetAttachedShapes().size(); j++)
		{
			ShapePairs.push_back(ShapeCollisionPair(first->GetAttachedShapes()[i], second->GetAttachedShapes()[j]));
		}
	}
}

void TD::ActorCollisionPair::Reset()
{
	for (int i = 0; i < ShapePairs.size(); i++)
	{
		ShapePairs[i].Data.Reset();
	}
}

void TD::RaycastData::AddContact(glm::vec3 point, glm::vec3 normal, float depth, TDShape* shape)
{
	BlockingHit = true;
	Points[Count].Distance = depth;
	Points[Count].Point = point;
	Points[Count].Normal = normal;
	Points[Count].Shape = shape;
	Count++;
}

bool FooPred(const TD::Contact& first, const TD::Contact& second)
{
	if (first.Distance < second.Distance)
	{
		return true;
	}
	return false;
}
void TD::RaycastData::SortClosest()
{
	std::sort(Points, Points + Count, FooPred);
}

void TD::RaycastData::Reset()
{
	Points[MAX_CONTACT_POINTS_COUNT];
	BlockingHit = false;
	Count = 0;
}

TD::ShapeCollisionPair::ShapeCollisionPair(TDShape * a, TDShape * b)
{
	A = a;
	B = b;
	AOwner = A->GetOwner();
	BOwner = B->GetOwner();
	IsTriggerPair = A->GetFlags().GetFlagValue(TDShapeFlags::ETrigger) || B->GetFlags().GetFlagValue(TDShapeFlags::ETrigger);
	SimPair = A->GetFlags().GetFlagValue(TDShapeFlags::ESimulation) && B->GetFlags().GetFlagValue(TDShapeFlags::ESimulation);
	if (!SimPair && IsTriggerPair)
	{
		if (A->GetFlags().GetFlagValue(TDShapeFlags::ETrigger) && B->GetFlags().GetFlagValue(TDShapeFlags::ETrigger))
		{
			IsTriggerPair = false;
		}
	}
}

bool TD::ShapeCollisionPair::IsPairValidForTrigger()
{
	if (!IsTriggerPair)
	{
		return false;
	}
	const bool IsAReady = A->GetFlags().GetFlagValue(TDShapeFlags::ETrigger) || A->GetFlags().GetFlagValue(TDShapeFlags::ESimulation);
	const bool IsBReady = B->GetFlags().GetFlagValue(TDShapeFlags::ETrigger) || B->GetFlags().GetFlagValue(TDShapeFlags::ESimulation);
	return IsAReady && IsBReady;
}

bool TD::RayCast::PreFilter(TDActor* actor, TDShape* Shape)
{
	if (InterSectionFilter != nullptr)
	{
		return InterSectionFilter->PreFilter(actor, Shape, this);
	}
	return true;
}

bool TD::RayCast::PostFilter()
{
	if (InterSectionFilter != nullptr)
	{
		return InterSectionFilter->PostFilter(HitData, this);
	}
	return true;
}
